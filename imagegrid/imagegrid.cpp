/**
 * Implementation of the main classes representing the grid.  Includes
 * both loaded images and (zoomed) textures.
 */
// local headers
#include "../common.hpp"
#include "../datatypes/coordinates.hpp"
#include "../utility.hpp"
#include "gridsetup.hpp"
#include "imagegrid.hpp"
#include "iterators.hpp"
#include "../viewport_current_state.hpp"
#include "imagegrid_load_file_data.hpp"
// C compatible headers
#include "../c_io_net/fileload.hpp"
// C++ headers
#include <atomic>
#include <fstream>
#include <string>
#include <utility>
#include <vector>
// C headers
#include <cmath>
#include <climits>

ImageGridSquareZoomLevel::ImageGridSquareZoomLevel(ImageGridSquare* parent_square,
                                                   INT64 zoom_out_shift) {
  this->_parent_square=parent_square;
  this->_max_sub_size=this->_parent_square->_max_sub_size >> zoom_out_shift;
  this->_zoom_out_shift=zoom_out_shift;
  this->_rgba_wpixel.init(this->sub_size());
  this->_rgba_hpixel.init(this->sub_size());
  this->_rgba_xpixel_origin.init(this->sub_size());
  this->_rgba_ypixel_origin.init(this->sub_size());
  // TODO: not yet
  this->_rgba_data.init(this->sub_size());
}

ImageGridSquareZoomLevel::~ImageGridSquareZoomLevel() {
  this->unload_square();
}

bool ImageGridSquareZoomLevel::load_square(ImageGridSquare* grid_square,
                                           bool use_cache,
                                           const std::vector<ImageGridSquareZoomLevel*>& dest_squares,
                                           INT64* row_temp_buffer) {
  bool load_successful=true;
  // iterate over square data
  LoadFileData file_data;
  LoadFileDataTransfer data_transfer;
  auto sub_w=grid_square->sub_size().w();
  auto sub_h=grid_square->sub_size().h();
  data_transfer.sub_size=SubGridImageSize(grid_square->sub_size());
  auto sub_size=sub_w*sub_h;
  data_transfer.original_rgba_wpixel.init(grid_square->sub_size());
  data_transfer.original_rgba_hpixel.init(grid_square->sub_size());
  for (INT64 sub_i_arr=0; sub_i_arr < sub_size; sub_i_arr++) {
    auto subgrid_index=SubGridIndex(sub_i_arr%sub_w,sub_i_arr/sub_w);
    data_transfer.original_rgba_wpixel.set(subgrid_index,grid_square->_subimages_wpixel[subgrid_index]);
    data_transfer.original_rgba_hpixel.set(subgrid_index,grid_square->_subimages_hpixel[subgrid_index]);
  }
  for (auto& dest_square : dest_squares) {
    file_data.data_pairs.emplace_back(std::pair<ImageGridSquareZoomLevel* const,
                                      std::shared_ptr<LoadFileZoomLevelData>>(dest_square,std::make_shared<LoadFileZoomLevelData>()));
    file_data.data_pairs.back().second->zoom_out_shift=dest_square->zoom_out_shift();
    file_data.data_pairs.back().second->max_sub_wpixel=dest_square->max_sub_pixel_size().w();
    file_data.data_pairs.back().second->max_sub_hpixel=dest_square->max_sub_pixel_size().h();
    data_transfer.data_transfer.emplace_back(file_data.data_pairs.back().second);
  }
  // TODO: need a more automatic solution to initialization like this
  //       these constructions are necessary because I want to load
  //       all data for a square before transfering
  // TODO: since these are fixed size, I could just allocate a
  //       transfer structure per square
  for (const auto& data_transfer_temp : data_transfer.data_transfer) {
    data_transfer_temp->rgba_data.init(grid_square->sub_size());
    data_transfer_temp->rgba_wpixel.init(grid_square->sub_size());
    data_transfer_temp->rgba_hpixel.init(grid_square->sub_size());
    for(const auto& subgrid_index : ImageSubGridBasicIterator(grid_square->_grid_setup,
                                                          *grid_square->grid_index())) {
      if (grid_square->grid_setup()->subgrid_has_data(grid_square->_grid_index,
                                                      subgrid_index)) {
        data_transfer_temp->rgba_data.set(subgrid_index,nullptr);
        data_transfer_temp->rgba_wpixel.set(subgrid_index,INT_MIN);
        data_transfer_temp->rgba_hpixel.set(subgrid_index,INT_MIN);
      }
    }
  }
  std::string cached_filename;
  for(const auto& subgrid_index : ImageSubGridBasicIterator(grid_square->_grid_setup,
                                                        *grid_square->grid_index())) {

    if (grid_square->grid_setup()->subgrid_has_data(grid_square->_grid_index,
                                                    subgrid_index)) {
      auto filename=grid_square->grid_setup()->filename(grid_square->_grid_index,subgrid_index);
      if (use_cache) {
        cached_filename=create_cache_filename(filename,"png");
      } else {
        // TODO: something better for invalid cached filename
        cached_filename="";
      }
      // TODO: is this line necessary?
      auto current_subgrid=SubGridIndex(subgrid_index);
      auto load_successful_temp=load_data_as_rgba(filename,
                                                  cached_filename,
                                                  current_subgrid,
                                                  data_transfer,
                                                  row_temp_buffer);
      // TODO: what happens if one part loads succesfully and
      // another does not? Not loading when I think it should load
      // is also pretty severe, this is probably a fatal error
      if (!load_successful_temp) {
        load_successful=false;
      }
    }
  }
  // TODO: change this once error handling is better
  if (load_successful) {
    for (auto& data_pair : file_data.data_pairs) {
      std::lock_guard<std::mutex> guard(data_pair.first->load_mutex);
      for(const auto& subgrid_index : ImageSubGridBasicIterator(grid_square->_grid_setup,
                                                            *grid_square->grid_index())) {
        auto sub_i=subgrid_index.i();
        auto sub_j=subgrid_index.j();
        if (grid_square->grid_setup()->subgrid_has_data(grid_square->_grid_index,
                                                        subgrid_index)) {
          auto origin_x=sub_i*(data_pair.first->_max_sub_size.w());
          auto origin_y=sub_j*(data_pair.first->_max_sub_size.w());
          data_pair.first->_rgba_wpixel.set(subgrid_index,data_pair.second->rgba_wpixel[subgrid_index]);
          data_pair.first->_rgba_hpixel.set(subgrid_index,data_pair.second->rgba_hpixel[subgrid_index]);
          data_pair.first->_rgba_xpixel_origin.set(subgrid_index,origin_x);
          data_pair.first->_rgba_ypixel_origin.set(subgrid_index,origin_y);
          data_pair.first->_rgba_data.set(subgrid_index,data_pair.second->rgba_data[subgrid_index]);
        }
      }
      data_pair.first->is_loaded=true;
    }
  }
  return load_successful;
}

void ImageGridSquareZoomLevel::unload_square() {
  if (this->is_loaded) {
    std::lock_guard<std::mutex> guard(this->load_mutex);
    this->is_loaded=false;

    for(const auto& subgrid_index : ImageSubGridBasicIterator(this->_parent_square->_grid_setup,
                                                          *this->_parent_square->grid_index())) {
      if (this->_rgba_data[subgrid_index]) {
        delete[] this->_rgba_data[subgrid_index];
        this->_rgba_data.set(subgrid_index,nullptr);
      }
    }
  }
}

INT64 ImageGridSquareZoomLevel::zoom_out_shift() const {
  return this->_zoom_out_shift;
}

INT64 ImageGridSquareZoomLevel::rgba_wpixel(const SubGridIndex& subgrid_index) const {
  return this->_rgba_wpixel[subgrid_index];
}

INT64 ImageGridSquareZoomLevel::rgba_hpixel(const SubGridIndex& subgrid_index) const {
  return this->_rgba_hpixel[subgrid_index];
}

INT64 ImageGridSquareZoomLevel::rgba_xpixel_origin(const SubGridIndex& subgrid_index) const {
  return this->_rgba_xpixel_origin[subgrid_index];
}

INT64 ImageGridSquareZoomLevel::rgba_ypixel_origin(const SubGridIndex& subgrid_index) const {
  return this->_rgba_ypixel_origin[subgrid_index];
}

PIXEL_RGBA* ImageGridSquareZoomLevel::rgba_data(const SubGridIndex& subgrid_index) const {
  return this->_rgba_data[subgrid_index];
}

SubGridImageSize ImageGridSquareZoomLevel::sub_size() const {
  return this->_parent_square->sub_size();
}

GridPixelSize ImageGridSquareZoomLevel::max_sub_pixel_size() const {
  return this->_max_sub_size;
}

ImageGridSquare* ImageGridSquareZoomLevel::parent_square() {
  return this->_parent_square;
}

ImageGridSquare::ImageGridSquare(GridSetup* grid_setup,
                                 ImageGrid* parent_grid,
                                 const GridIndex& grid_index) {
  this->_grid_setup=grid_setup;
  this->_parent_grid=parent_grid;
  this->_grid_index=GridIndex(grid_index);
  this->_subimages_wpixel.init(this->sub_size());
  this->_subimages_hpixel.init(this->sub_size());
  auto read_successful=this->_read_data();
  if (!read_successful) {
    this->_status=ImageGridStatus::load_error;
  } else {
    this->_status=ImageGridStatus::loading;
  }
}

bool ImageGridSquare::_read_data() {
  // default is 1x1 unless otherwise read
  INT64 max_sub_wpixel=1;
  INT64 max_sub_hpixel=1;
  INT64 sub_w=this->_grid_setup->sub_size(this->_grid_index).w();
  INT64 sub_h=this->_grid_setup->sub_size(this->_grid_index).h();
  auto successful=true;
    for(const auto& subgrid_index : ImageSubGridBasicIterator(this->_grid_setup,
                                                          *this->grid_index())) {
      INT64 image_wpixel;
      INT64 image_hpixel;
      if (this->grid_setup()->subgrid_has_data(this->_grid_index,
                                               subgrid_index)) {
        auto filename=this->_grid_setup->filename(this->_grid_index,
                                                  subgrid_index);
        // read raw data if no cache or cache unsuccessful
        auto this_successful=read_data(filename,
                                       this->grid_setup()->use_cache(),
                                       image_wpixel,
                                       image_hpixel);
        if (!this_successful) {
          successful=false;
        }
        this->_subimages_wpixel.set(subgrid_index,image_wpixel);
        this->_subimages_hpixel.set(subgrid_index,image_hpixel);
        if (image_wpixel > max_sub_wpixel) {
          max_sub_wpixel=image_wpixel;
        }
        if (image_hpixel > max_sub_hpixel) {
          max_sub_hpixel=image_hpixel;
        }
      }
  }
  this->_square_size=GridPixelSize(max_sub_wpixel*sub_w,
                                    max_sub_hpixel*sub_h);
  this->_max_sub_size=GridPixelSize(max_sub_wpixel,
                                    max_sub_hpixel);
  return successful;
}


GridSetup* ImageGridSquare::grid_setup() const {
  return this->_grid_setup;
}

SubGridImageSize ImageGridSquare::sub_size() const {
  return SubGridImageSize(this->grid_setup()->sub_size(this->_grid_index).w(),
                          this->grid_setup()->sub_size(this->_grid_index).h());
}

ImageGrid* ImageGridSquare::parent_grid() const {
  return this->_parent_grid;
}

const GridIndex* ImageGridSquare::grid_index() const {
  return &this->_grid_index;
}

void ImageGrid::read_grid_info(GridSetup* grid_setup, std::shared_ptr<ViewPortTransferState> viewport_current_state_imagegrid_update) {
  this->_status=ImageGridStatus::loading;
  this->_grid_setup=grid_setup;
  this->_viewport_current_state_imagegrid_update=viewport_current_state_imagegrid_update;
  this->_read_grid_info_setup_squares(grid_setup);
}

ImageGridStatus ImageGrid::status () const {
  return this->_status;
}

void ImageGrid::_read_grid_info_setup_squares(GridSetup* const grid_setup) {
  // delayed allocation for the squares
  this->_squares.init(grid_setup->grid_image_size());
  this->_image_max_size=GridPixelSize(0,0);
  // not ready for an iterator until full sparsity is implemented and tested
  INT64 new_wpixel=INT_MIN;
  INT64 new_hpixel=INT_MIN;
  for (const auto& grid_index : ImageGridBasicIterator(this->_grid_setup)) {
    // [square_index]=std::make_unique<ImageGridSquare>(grid_setup,this,GridIndex(grid_index));
    this->_squares.set(grid_index,std::make_unique<ImageGridSquare>(grid_setup,this,GridIndex(grid_index)));
    // TODO: not skipping rest for now, just setting as load error
    if (this->_squares[grid_index]->_status == ImageGridStatus::load_error) {
      this->_status=ImageGridStatus::load_error;
    }
    // set the RGBA of the surface
    auto rgba_wpixel=this->_squares[grid_index]->_square_size.w();
    auto rgba_hpixel=this->_squares[grid_index]->_square_size.h();
    if ((INT64)rgba_wpixel > new_wpixel) {
      new_wpixel=(INT64)rgba_wpixel;
    }
    if ((INT64)rgba_hpixel > new_hpixel) {
      new_hpixel=(INT64)rgba_hpixel;
    }
  }
  this->_image_max_size=GridPixelSize(new_wpixel,new_hpixel);;
  auto image_max_size_wpixel=this->_image_max_size.w();
  auto image_max_size_hpixel=this->_image_max_size.h();
  // allocate temporary buffers to use as a working area
  this->_row_temp_buffer=std::make_unique<INT64[]>(new_wpixel*3);
  // find how many zoom_out_shifts to get whole image grid as a 3x3 grid of original size
  // TODO: revise description of why this works
  auto max_scale=(INT64)ceil((FLOAT64)(fmax((FLOAT64)image_max_size_wpixel,(FLOAT64)image_max_size_hpixel))/(FLOAT64)MAX_MIN_SCALED_IMAGE_SIZE);
  this->_max_zoom_out_shift=(INT64)ceil(log2(max_scale));
  if (this->_max_zoom_out_shift < 1) {
    this->_max_zoom_out_shift=1;
  }
  // diagnostic information
  MSG("max_scale: " << max_scale);
  MSG("max_zoom_out_shift: " << this->_max_zoom_out_shift);
  MSG("max_wpixel: " << image_max_size_wpixel);
  MSG("max_hpixel: " << image_max_size_hpixel);
  // add this info to the various data structure
  for (const auto& grid_index : ImageGridBasicIterator(grid_setup)) {
    this->_squares[grid_index]->image_array.init(this->_max_zoom_out_shift);
    // this->_squares[grid_index]->image_array=std::make_unique<std::unique_ptr<ImageGridSquareZoomLevel>[]>(this->_max_zoom_out_shift);
    INT64 zoom_out_shift=0;
    for (auto k=0L; k < this->_max_zoom_out_shift; k++) {
      this->_squares[grid_index]->image_array.set(k,std::make_unique<ImageGridSquareZoomLevel>(this->_squares[grid_index],
                                                                                               zoom_out_shift));
      zoom_out_shift+=1;
    }
    // if not error set square as loaded
    if (this->_squares[grid_index]->_status != ImageGridStatus::load_error) {
      this->_squares[grid_index]->_status = ImageGridStatus::loaded;
    }
  }
  if (this->_status != ImageGridStatus::load_error) {
    this->_status=ImageGridStatus::loaded;
  }
  ////////////////////////////////////////////////////////////////////////////////
  // testing range-based for loops
  // for (const auto& grid_index : ImageGridBasicIterator(grid_setup)) {
  //   MSG("================================================================================");
  //   MSG(grid_index.i());
  //   MSG(grid_index.j());
  //   for (const auto& subgrid_index : ImageSubGridBasicIterator(grid_setup,grid_index)) {
  //     MSG("--------------------");
  //     MSG(subgrid_index.i());
  //     MSG(subgrid_index.j());
  //   }
  // }
}

INT64 ImageGrid::max_zoom_out_shift() const {
  return this->_max_zoom_out_shift;
}

bool ImageGrid::_check_bounds(const GridIndex* grid_index) const {
  auto i=grid_index->i();
  auto j=grid_index->j();
  auto return_value=(i >= 0 && i < this->grid_image_size().w() && j >= 0 && j < this->grid_image_size().h());
  return return_value;
}

bool ImageGrid::_check_load(const ViewPortCurrentState& viewport_current_state,
                            INT64 zoom_out_shift,
                            const GridIndex* grid_index,
                            INT64 zoom_out_shift_lower_limit,
                            INT64 load_all) {
  auto i=grid_index->i();
  auto j=grid_index->j();
  auto current_grid_x=viewport_current_state.current_grid_coordinate().x();
  auto current_grid_y=viewport_current_state.current_grid_coordinate().y();
  auto viewport_current_state_new=ViewPortCurrentState(viewport_current_state.current_grid_coordinate(),
                                                       this->_image_max_size,
                                                       ViewPortTransferState::find_zoom_upper(zoom_out_shift),
                                                       viewport_current_state.screen_size(),
                                                       BufferPixelCoordinate(0,0),
                                                       BufferPixelCoordinate(0,0));
  auto return_value=((zoom_out_shift == this->_max_zoom_out_shift-1 ||
                      (zoom_out_shift >= zoom_out_shift_lower_limit &&
                       ViewPortTransferState::grid_index_visible(i,j,
                                                                 viewport_current_state_new)) ||
                      // only load adjacent grid below zoom limit
                      (i >= (floor(current_grid_x)-1) && i <= (floor(current_grid_x)+1) &&
                       j >= (floor(current_grid_y)-1) && j <= (floor(current_grid_y)+1)) ||
                      load_all));
  return return_value;
}

bool ImageGrid::_load_square(const ViewPortCurrentState& viewport_current_state,
                             const GridIndex* grid_index,
                             INT64 zoom_out_shift_lower_limit,
                             INT64 load_all, const GridSetup* const grid_setup) {
  // always load if top level
  bool tried_load=false;
  bool never_false=true;
  if (this->_check_bounds(grid_index)) {
    std::vector<INT64> zoom_out_shift_list;
    for (INT64 zoom_out_shift=0; zoom_out_shift < this->_max_zoom_out_shift; zoom_out_shift++) {
      if (this->_check_load(viewport_current_state,
                            zoom_out_shift,
                            grid_index,
                            zoom_out_shift_lower_limit,
                            load_all)) {
        if (!this->squares(grid_index)->image_array[zoom_out_shift]->is_loaded) {
          zoom_out_shift_list.push_back(zoom_out_shift);
        }
      }
    }
    if (zoom_out_shift_list.size() > 0) {
      auto dest_squares=std::vector<ImageGridSquareZoomLevel*>{};
      for (const auto& zoom_out_shift_item : zoom_out_shift_list) {
        dest_squares.push_back(this->squares(grid_index)->image_array[zoom_out_shift_item]);
      }
      tried_load=true;
      auto load_successful_temp=ImageGridSquareZoomLevel::load_square(this->squares(grid_index),
                                                                      grid_setup->use_cache(),
                                                                      dest_squares,
                                                                      this->_row_temp_buffer.get());
      if (!load_successful_temp) {
        never_false=false;
      }
    }
  }
  return (tried_load && never_false);
}

void ImageGrid::_write_cache(const GridIndex& grid_index) {
  auto sub_w=this->_grid_setup->sub_size(grid_index).w();
  auto sub_h=this->_grid_setup->sub_size(grid_index).h();
  for (const auto& subgrid_index : ImageSubGridBasicIterator(this->_grid_setup,
                                                         grid_index)) {
    auto sub_i=subgrid_index.i();
    auto sub_j=subgrid_index.j();
      bool loaded_cache_size=false;
      // TODO: probaby not optimal place to find this
      auto dest_square_zero=this->squares(grid_index)->image_array[0];
      auto full_wpixel=dest_square_zero->rgba_wpixel(subgrid_index);
      auto full_hpixel=dest_square_zero->rgba_hpixel(subgrid_index);
      for (INT64 k=0L; k<this->_max_zoom_out_shift; k++) {
        if (loaded_cache_size) {
          break;
        }
        auto dest_square=this->squares(grid_index)->image_array[k];
        // find current size
        auto wpixel=dest_square->rgba_wpixel(subgrid_index);
        auto hpixel=dest_square->rgba_hpixel(subgrid_index);
        auto filename=this->grid_setup()->filename(grid_index,subgrid_index);
        if (check_valid_filename(filename)) {
          // TODO: combine these
          auto filename_png=create_cache_filename(filename,"png");
          auto filename_txt=create_cache_filename(filename,"txt");
          MSG("Trying to writing cache filenames: " <<
              filename_png << " " <<  " " << filename_txt <<
              " for " << filename << " at zoom index " << k <<
              " i: " << grid_index.i() << " j: " << grid_index.j() <<
              " sub_i: " << sub_i << " sub_j: " << sub_j);
          if (sub_w*wpixel < CACHE_MAX_PIXEL_SIZE && sub_h*hpixel < CACHE_MAX_PIXEL_SIZE) {
            loaded_cache_size=write_png_text(filename_png, filename_txt,
                                             wpixel, hpixel,
                                             full_wpixel, full_hpixel,
                                             dest_square->_rgba_data[subgrid_index]);
            MSG("Cache tried with return: " << loaded_cache_size);
            if (loaded_cache_size) {
              MSG("Cached worked with w: " << wpixel << " h: " << hpixel);
            }
          }
        }
      }
  }
}

// ImageGridSquare* ImageGrid::_get_squares(const GridIndex& grid_index) {
//   return this->_squares[grid_index].get();
//   // return this->_squares[grid_index.j()*this->_grid_setup->grid_image_size().w()+grid_index.i()].get();
// }

void ImageGrid::load_grid(const GridSetup* const grid_setup, std::atomic<bool>& keep_running) {
  auto keep_trying=true;
  // get information on viewport
  auto viewport_current_state=this->_viewport_current_state_imagegrid_update->GetGridValues();
  // find the grid extents and choose things that should be loaded/unloaded
  // different things for what should be loaded/unloaded
  // load in reverse order of size
  auto current_zoom_out_shift=ViewPortTransferState::find_zoom_out_shift_bounded(viewport_current_state.zoom(),0.0,this->_max_zoom_out_shift-1);
  auto load_all=false;
  auto zoom_out_shift_lower_limit=current_zoom_out_shift-1;
  // unload first
  for (auto zoom_out_shift=this->_max_zoom_out_shift-1; zoom_out_shift >= 0L; zoom_out_shift--) {
    for (const auto& grid_index : ImageGridBasicIterator(this->_grid_setup)) {
      if (!keep_running) {
        keep_trying=false;
      }
      if (!this->_check_load(viewport_current_state,
                             zoom_out_shift, &grid_index, zoom_out_shift_lower_limit,
                             load_all)) {
        this->_squares[grid_index]->image_array[zoom_out_shift]->unload_square();
        // always try and unload rest, except top level
      }
    }
  }
  // files actually loaded
  INT64 load_count=0;
  auto iterator_visible=this->grid_setup()->iterator_visible(viewport_current_state);
  // we are looking at if things are not loaded
  while (keep_trying && load_count < LOAD_FILES_BATCH && keep_running ) {
    auto grid_index=iterator_visible->get_next();
    keep_trying=(!grid_index->invalid());
    if (keep_trying) {
      if (this->_check_bounds(grid_index.get()) && grid_setup->square_has_data(grid_index.get())) {

        auto load_successful=this->_load_square(viewport_current_state,
                                                grid_index.get(),
                                                zoom_out_shift_lower_limit,
                                                load_all, grid_setup);
        if (load_successful) { load_count++; }
      }
    }
  }
  keep_trying=(load_count < LOAD_FILES_BATCH && keep_running);
  if (keep_trying) {
    auto iterator_normal=this->grid_setup()->iterator_full(viewport_current_state);
    while (keep_trying && load_count < LOAD_FILES_BATCH && keep_running) {
      auto grid_index=iterator_normal->get_next();
      keep_trying=(!grid_index->invalid());
      if (keep_trying) {
        if (this->_check_bounds(grid_index.get()) && grid_setup->square_has_data(grid_index.get())) {
          auto load_successful=this->_load_square(viewport_current_state,
                                                  grid_index.get(),
                                                  zoom_out_shift_lower_limit,
                                                  load_all, grid_setup);
          if (load_successful) { load_count++; }
        }
      }
    }
  }
}

GridPixelSize ImageGrid::image_max_pixel_size() const {
  return this->_image_max_size;
}

ImageGridSquare* ImageGrid::squares(const GridIndex& grid_index) {
  return this->_squares[grid_index];
}

ImageGridSquare* ImageGrid::squares(const GridIndex* grid_index) {
  return this->_squares[*grid_index];
}

void ImageGrid::setup_grid_cache(GridSetup* const grid_setup) {
  // no read_grid_info(...) called for now
  this->_read_grid_info_setup_squares(grid_setup);
  // loop over the whole grid
  for (const auto& grid_index : ImageGridBasicIterator(grid_setup)) {
    // load the file into the data structure
    // this is a dummy ViewPortCurrentState
    this->_load_square(ViewPortCurrentState(GridCoordinate(0.0,0.0),
                                            GridPixelSize(0,0),
                                            0.0,
                                            BufferPixelSize(0,0),
                                            BufferPixelCoordinate(0,0),
                                            BufferPixelCoordinate(0,0)),
                       &grid_index,
                       0L,true,grid_setup);
    // TODO: eventually cache this out as tiles that fit in 128x128 and 512x512
    this->_write_cache(grid_index);
    // unload
    for (auto k=this->_max_zoom_out_shift-1; k >= 0L; k--) {
      this->_squares[grid_index]->image_array[k]->unload_square();
    }
  }
}

GridSetup* ImageGrid::grid_setup() const {
  return this->_grid_setup;
}

const GridImageSize ImageGrid::grid_image_size() const {
  return this->_grid_setup->grid_image_size();
}
