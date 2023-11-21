/**
 * Implementation of the main classes representing the grid.  Includes
 * both loaded images and (zoomed) textures.
 */
// local headers
#include "../common.hpp"
#include "../coordinates.hpp"
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
                                                   INT64 zoom_out) {
  this->_parent_square=parent_square;
  this->_max_sub_size=this->_parent_square->_max_sub_size/zoom_out;
  this->_zoom_out=zoom_out;
  this->_rgba_wpixel=std::make_unique<INT64[]>(this->sub_size().w()*this->sub_size().h());
  this->_rgba_hpixel=std::make_unique<INT64[]>(this->sub_size().w()*this->sub_size().h());
  this->_rgba_xpixel_origin=std::make_unique<INT64[]>(this->sub_size().w()*this->sub_size().h());
  this->_rgba_ypixel_origin=std::make_unique<INT64[]>(this->sub_size().w()*this->sub_size().h());
  this->_rgba_data=std::make_unique<PIXEL_RGBA*[]>(this->sub_size().w()*this->sub_size().h());
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
  data_transfer.original_rgba_wpixel=std::make_unique<size_t[]>(sub_size);
  data_transfer.original_rgba_hpixel=std::make_unique<size_t[]>(sub_size);
  for (INT64 sub_i_arr=0; sub_i_arr < sub_size; sub_i_arr++) {
    data_transfer.original_rgba_wpixel[sub_i_arr]=grid_square->_subimages_wpixel[sub_i_arr];
    data_transfer.original_rgba_hpixel[sub_i_arr]=grid_square->_subimages_hpixel[sub_i_arr];
  }
  for (auto& dest_square : dest_squares) {
    file_data.data_pairs.emplace_back(std::pair<ImageGridSquareZoomLevel* const,
                                      std::shared_ptr<LoadFileZoomLevelData>>(dest_square,std::make_shared<LoadFileZoomLevelData>()));
    file_data.data_pairs.back().second->zoom_out=dest_square->zoom_out();
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
    data_transfer_temp->rgba_data=std::make_unique<PIXEL_RGBA*[]>(sub_size);
    data_transfer_temp->rgba_wpixel=std::make_unique<INT64[]>(sub_size);
    data_transfer_temp->rgba_hpixel=std::make_unique<INT64[]>(sub_size);
    for (INT64 sub_j=0; sub_j < sub_h; sub_j++) {
      for (INT64 sub_i=0; sub_i < sub_w; sub_i++) {
        auto sub_index=SubGridIndex(sub_i,sub_j);
        if (grid_square->grid_setup()->subgrid_has_data(grid_square->_grid_index,
                                                        sub_index)) {
          auto sub_i_arr=sub_j*sub_w+sub_i;
          data_transfer_temp->rgba_data[sub_i_arr]=nullptr;
          data_transfer_temp->rgba_wpixel[sub_i_arr]=INT_MIN;
          data_transfer_temp->rgba_hpixel[sub_i_arr]=INT_MIN;
        }
      }
    }
  }
  std::string cached_filename;
  for (INT64 sub_j=0; sub_j < sub_h; sub_j++) {
    for (INT64 sub_i=0; sub_i < sub_w; sub_i++) {
      auto sub_index=SubGridIndex(sub_i,sub_j);
      if (grid_square->grid_setup()->subgrid_has_data(grid_square->_grid_index,
                                                      sub_index)) {
        auto filename=grid_square->grid_setup()->get_filename(grid_square->_grid_index,sub_index);
        if (use_cache) {
          cached_filename=create_cache_filename(filename,"png");
        } else {
          // TODO: something better for invalid cached filename
          cached_filename="";
        }
        auto current_subgrid=SubGridIndex(sub_i,sub_j);
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
  }
  // TODO: change this once error handling is better
  if (load_successful) {
    for (auto& data_pair : file_data.data_pairs) {
      std::lock_guard<std::mutex> guard(data_pair.first->load_mutex);
      for (INT64 sub_i=0; sub_i < sub_w; sub_i++) {
        for (INT64 sub_j=0; sub_j < sub_h; sub_j++) {
          auto sub_index=SubGridIndex(sub_i,sub_j);
          if (grid_square->grid_setup()->subgrid_has_data(grid_square->_grid_index,
                                                          sub_index)) {
            auto origin_x=sub_i*(data_pair.first->_max_sub_size.w());
            auto origin_y=sub_j*(data_pair.first->_max_sub_size.w());
            auto sub_i_arr=sub_j*sub_w+sub_i;
            data_pair.first->_rgba_wpixel[sub_i_arr]=data_pair.second->rgba_wpixel[sub_i_arr];
            data_pair.first->_rgba_hpixel[sub_i_arr]=data_pair.second->rgba_hpixel[sub_i_arr];
            data_pair.first->_rgba_xpixel_origin[sub_i_arr]=origin_x;
            data_pair.first->_rgba_ypixel_origin[sub_i_arr]=origin_y;
            data_pair.first->_rgba_data[sub_i_arr]=data_pair.second->rgba_data[sub_i_arr];
          }
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
    for (INT64 sub_i=0; sub_i < this->_parent_square->sub_size().w(); sub_i++) {
      for (INT64 sub_j=0; sub_j < this->_parent_square->sub_size().h(); sub_j++) {
        auto sub_i_arr=this->_sub_index_arr(sub_i,sub_j);
        if (this->_rgba_data[sub_i_arr]) {
          delete[] this->_rgba_data[sub_i_arr];
          this->_rgba_data[sub_i_arr]=nullptr;
        }
      }
    }
  }
}

INT64 ImageGridSquareZoomLevel::zoom_out_shift() const {
  // TODO: eliminate this float calculation once shifts are used to
  // indicate zoom everywhere
  return (INT64)floor(log2(this->_zoom_out));
}

INT64 ImageGridSquareZoomLevel::zoom_out() const {
  return this->_zoom_out;
}

INT64 ImageGridSquareZoomLevel::rgba_wpixel(const SubGridIndex& sub_index) const {
  return this->_rgba_wpixel[this->_sub_index_arr(sub_index)];
}

INT64 ImageGridSquareZoomLevel::rgba_hpixel(const SubGridIndex& sub_index) const {
  return this->_rgba_hpixel[this->_sub_index_arr(sub_index)];
}

INT64 ImageGridSquareZoomLevel::rgba_xpixel_origin(const SubGridIndex& sub_index) const {
  return this->_rgba_xpixel_origin[this->_sub_index_arr(sub_index)];
}

INT64 ImageGridSquareZoomLevel::rgba_ypixel_origin(const SubGridIndex& sub_index) const {
  return this->_rgba_ypixel_origin[this->_sub_index_arr(sub_index)];
}

PIXEL_RGBA* ImageGridSquareZoomLevel::get_rgba_data(const SubGridIndex& sub_index) const {
  return this->_rgba_data[this->_sub_index_arr(sub_index)];
}

SubGridImageSize ImageGridSquareZoomLevel::sub_size() const {
  return this->_parent_square->sub_size();
}

// INT64 ImageGridSquareZoomLevel::sub_w() const {
//   return this->_parent_square->sub_w();
// }
//
// INT64 ImageGridSquareZoomLevel::sub_h() const {
//   return this->_parent_square->sub_h();
// }

GridPixelSize ImageGridSquareZoomLevel::max_sub_pixel_size() const {
  return this->_max_sub_size;
}

// INT64 ImageGridSquareZoomLevel::max_sub_wpixel() const {
//   return this->_max_sub_size.w();
// }
//
// INT64 ImageGridSquareZoomLevel::max_sub_hpixel() const {
//   return this->_max_sub_size.h();
// }

ImageGridSquare* ImageGridSquareZoomLevel::parent_square() {
  return this->_parent_square;
}

INT64 ImageGridSquareZoomLevel::_sub_index_arr(INT64 sub_i, INT64 sub_j) const {
  return sub_j*this->sub_size().w()+sub_i;
}

INT64 ImageGridSquareZoomLevel::_sub_index_arr(const SubGridIndex& sub_index) const {
  return sub_index.j()*this->sub_size().w()+sub_index.i();
}

ImageGridSquare::ImageGridSquare(GridSetup* grid_setup,
                                 ImageGrid* parent_grid,
                                 const GridIndex& grid_index) {
  this->_grid_setup=grid_setup;
  this->_parent_grid=parent_grid;
  this->_grid_index=GridIndex(grid_index);
  this->_subimages_wpixel=std::make_unique<INT64[]>(this->sub_size().w()*this->sub_size().h());
  this->_subimages_hpixel=std::make_unique<INT64[]>(this->sub_size().w()*this->sub_size().h());
  this->_read_data();
}

void ImageGridSquare::_read_data() {
  // default is 1x1 unless otherwise read
  INT64 max_sub_wpixel=1;
  INT64 max_sub_hpixel=1;
  INT64 sub_w=this->_grid_setup->sub_size(this->_grid_index).w();
  INT64 sub_h=this->_grid_setup->sub_size(this->_grid_index).h();

  for (INT64 sub_j=0; sub_j < sub_h; sub_j++) {
    for (INT64 sub_i=0; sub_i < sub_w; sub_i++) {
      INT64 image_wpixel;
      INT64 image_hpixel;
      auto sub_index=SubGridIndex(sub_i,sub_j);
      if (this->grid_setup()->subgrid_has_data(this->_grid_index,
                                               sub_index)) {
        auto filename=this->_grid_setup->get_filename(this->_grid_index,
                                                      sub_index);
        // read raw data if no cache or cache unsuccessful
        // TODO: check for success
        read_data(filename,
                  this->grid_setup()->use_cache(),
                  image_wpixel,
                  image_hpixel);
        auto sub_i_arr=sub_j*sub_w+sub_i;
        this->_subimages_wpixel[sub_i_arr]=image_wpixel;
        this->_subimages_hpixel[sub_i_arr]=image_hpixel;
        if (image_wpixel > max_sub_wpixel) {
          max_sub_wpixel=image_wpixel;
        }
        if (image_hpixel > max_sub_hpixel) {
          max_sub_hpixel=image_hpixel;
        }
      }
    }
  }
  this->_square_size=GridPixelSize(max_sub_wpixel*sub_w,
                                    max_sub_hpixel*sub_h);
  // this->_square_wpixel=max_sub_wpixel*sub_w;
  // this->_square_hpixel=max_sub_hpixel*sub_h;
  this->_max_sub_size=GridPixelSize(max_sub_wpixel,
                                    max_sub_hpixel);
  // this->_max_sub_wpixel=max_sub_wpixel;
  // this->_max_sub_hpixel=max_sub_hpixel;
}

void ImageGrid::_read_grid_info_setup_squares(GridSetup* const grid_setup) {
  // delayed allocation for the squares
  auto grid_wimage=grid_setup->grid_image_size().w();
  auto grid_himage=grid_setup->grid_image_size().h();
  this->_squares=std::make_unique<std::unique_ptr<std::unique_ptr<ImageGridSquare>[]>[]>(grid_wimage);
  for (INT64 i=0L; i < grid_setup->grid_image_size().w(); i++) {
    this->_squares[i]=std::make_unique<std::unique_ptr<ImageGridSquare>[]>(grid_himage);
  }
  this->_image_max_size=GridPixelSize(0,0);
  // not ready for an iterator until full sparsity is implemented and tested
  INT64 new_wpixel=INT_MIN;
  INT64 new_hpixel=INT_MIN;
  for (INT64 i=0; i<grid_wimage;i++) {
    for (INT64 j=0; j<grid_himage;j++) {
      this->_squares[i][j]=std::make_unique<ImageGridSquare>(grid_setup,this,GridIndex(i,j));
      // set the RGBA of the surface
      auto rgba_wpixel=this->_squares[i][j]->_square_size.w();
      auto rgba_hpixel=this->_squares[i][j]->_square_size.h();
      if ((INT64)rgba_wpixel > new_wpixel) {
        new_wpixel=(INT64)rgba_wpixel;
      }
      if ((INT64)rgba_hpixel > new_hpixel) {
        new_hpixel=(INT64)rgba_hpixel;
      }
    }
  }
  this->_image_max_size=GridPixelSize(new_wpixel,new_hpixel);;
  auto image_max_size_wpixel=this->_image_max_size.w();
  auto image_max_size_hpixel=this->_image_max_size.h();
  // allocate temporary buffers to use as a working area
  this->_row_temp_buffer=std::make_unique<INT64[]>(new_wpixel*3);
  // find how many zoom_step to get whole image grid as a 3x3 grid of original size
  // TODO: revise description of why this works
  auto max_scale=(INT64)ceil((FLOAT64)(fmax(image_max_size_wpixel,image_max_size_hpixel))/(FLOAT64)MAX_MIN_SCALED_IMAGE_SIZE);
  this->_zoom_index_length=(INT64)ceil(log2(max_scale));
  if (this->_zoom_index_length < 1) {
    this->_zoom_index_length=1;
  }
  // diagnostic information
  MSG("max_scale: " << max_scale);
  MSG("max_zoom_index: " << this->_zoom_index_length);
  auto zoom_step=ZOOM_STEP;
  MSG("zoom_step: " << zoom_step);
  MSG("max_wpixel: " << image_max_size_wpixel);
  MSG("max_hpixel: " << image_max_size_hpixel);
  // add this info to the various data structure
  for (INT64 i=0L; i < this->grid_image_size().w(); i++) {
    for (INT64 j=0L; j < this->grid_image_size().h(); j++) {
      this->_squares[i][j]->image_array=std::make_unique<std::unique_ptr<ImageGridSquareZoomLevel>[]>(this->_zoom_index_length);
      INT64 zoom_out=1;
      for (auto k=0L; k < this->_zoom_index_length; k++) {
        this->_squares[i][j]->image_array[k]=std::make_unique<ImageGridSquareZoomLevel>(this->_squares[i][j].get(),
                                                                                        zoom_out);
        zoom_out*=zoom_step;
      }
    }
  }
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
  this->_grid_setup=grid_setup;
  this->_viewport_current_state_imagegrid_update=viewport_current_state_imagegrid_update;
  this->_read_grid_info_setup_squares(grid_setup);
  this->_read_grid_info_successful=true;
}

bool ImageGrid::read_grid_info_successful() const {
  return this->_read_grid_info_successful;
}

INT64 ImageGrid::zoom_index_length() const {
  return this->_zoom_index_length;
}

bool ImageGrid::_check_bounds(const GridIndex* grid_index) const {
  auto i=grid_index->i();
  auto j=grid_index->j();
  auto return_value=(i >= 0 && i < this->grid_image_size().w() && j >= 0 && j < this->grid_image_size().h());
  return return_value;
}

bool ImageGrid::_check_load(const ViewPortCurrentState& viewport_current_state,
                            INT64 zoom_index,
                            const GridIndex* grid_index,
                            INT64 zoom_index_lower_limit,
                            INT64 load_all) {
  auto i=grid_index->i();
  auto j=grid_index->j();
  auto current_grid_x=viewport_current_state.current_grid_coordinate().x();
  auto current_grid_y=viewport_current_state.current_grid_coordinate().y();
  auto viewport_current_state_new=ViewPortCurrentState(viewport_current_state.current_grid_coordinate(),
                                                       this->_image_max_size,
                                                       ViewPortTransferState::find_zoom_upper(zoom_index),
                                                       viewport_current_state.screen_size(),
                                                       BufferPixelCoordinate(0,0),
                                                       BufferPixelCoordinate(0,0));
  auto return_value=((zoom_index == this->_zoom_index_length-1 ||
                      (zoom_index >= zoom_index_lower_limit &&
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
                             INT64 zoom_index_lower_limit,
                             INT64 load_all, const GridSetup* const grid_setup) {
  // always load if top level
  bool tried_load=false;
  bool never_false=true;
  if (this->_check_bounds(grid_index)) {
    std::vector<INT64> zoom_index_list;
    for (INT64 zoom_index=this->_zoom_index_length-1; zoom_index >= 0L; zoom_index--) {
      if (this->_check_load(viewport_current_state,
                            zoom_index,
                            grid_index,
                            zoom_index_lower_limit,
                            load_all)) {
        if (!this->squares(grid_index)->image_array[zoom_index]->is_loaded) {
          zoom_index_list.push_back(zoom_index);
        }
      }
    }
    if (zoom_index_list.size() > 0) {
      auto dest_squares=std::vector<ImageGridSquareZoomLevel*>{};
      for (const auto& zoom_index : zoom_index_list) {
        dest_squares.push_back(this->squares(grid_index)->image_array[zoom_index].get());
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
  for (INT64 sub_j=0; sub_j < sub_h; sub_j++) {
    for (INT64 sub_i=0; sub_i < sub_w; sub_i++) {
      auto sub_index=SubGridIndex(sub_i,sub_j);
      bool loaded_cache_size=false;
      // TODO: probaby not optimal place to find this
      auto dest_square_zero=this->squares(grid_index)->image_array[0].get();
      auto full_wpixel=dest_square_zero->rgba_wpixel(sub_index);
      auto full_hpixel=dest_square_zero->rgba_hpixel(sub_index);
      for (INT64 k=0L; k<this->_zoom_index_length; k++) {
        if (loaded_cache_size) {
          break;
        }
        auto dest_square=this->squares(grid_index)->image_array[k].get();
        // find current size
        auto wpixel=dest_square->rgba_wpixel(sub_index);
        auto hpixel=dest_square->rgba_hpixel(sub_index);
        auto filename=this->grid_setup()->get_filename(grid_index,sub_index);
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
            auto sub_i_arr=sub_j*sub_w+sub_i;
            loaded_cache_size=write_png_text(filename_png, filename_txt,
                                             wpixel, hpixel,
                                             full_wpixel, full_hpixel,
                                             dest_square->_rgba_data[sub_i_arr]);
            MSG("Cache tried with return: " << loaded_cache_size);
            if (loaded_cache_size) {
              MSG("Cached worked with w: " << wpixel << " h: " << hpixel);
            }
          }
        }
      }
    }
  }
}

void ImageGrid::load_grid(const GridSetup* const grid_setup, std::atomic<bool>& keep_running) {
  auto keep_trying=true;
  // get information on viewport
  auto viewport_current_state=this->_viewport_current_state_imagegrid_update->GetGridValues();
  // find the grid extents and choose things that should be loaded/unloaded
  // different things for what should be loaded/unloaded
  // load in reverse order of size
  auto current_zoom_index=ViewPortTransferState::find_zoom_index_bounded(viewport_current_state.zoom(),0.0,this->_zoom_index_length-1);
  auto grid_w=this->grid_image_size().w();
  auto grid_h=this->grid_image_size().h();
  auto load_all=false;
  auto zoom_index_lower_limit=current_zoom_index-1;
  // unload first
  for (auto zoom_index=this->_zoom_index_length-1; zoom_index >= 0L; zoom_index--) {
    for (INT64 i=0L; i < grid_w; i++) {
      for (INT64 j=0L; j < grid_h; j++) {
        if (!keep_running) {
          keep_trying=false;
        }
        auto grid_index=GridIndex(i,j);
        if (!this->_check_load(viewport_current_state,
                               zoom_index, &grid_index, zoom_index_lower_limit,
                               load_all)) {
          this->_squares[i][j]->image_array[zoom_index]->unload_square();
          // always try and unload rest, except top level
        }
      }
    }
  }
  // files actually loaded
  INT64 load_count=0;
  auto iterator_visible=this->grid_setup()->get_iterator_visible(viewport_current_state);
  // we are looking at if things are not loaded
  while (keep_trying && load_count < LOAD_FILES_BATCH && keep_running ) {
    auto grid_index=iterator_visible->get_next();
    keep_trying=(!grid_index->invalid());
    if (keep_trying) {
      if (this->_check_bounds(grid_index.get()) && grid_setup->square_has_data(grid_index.get())) {

        auto load_successful=this->_load_square(viewport_current_state,
                                                grid_index.get(),
                                                zoom_index_lower_limit,
                                                load_all, grid_setup);
        if (load_successful) { load_count++; }
      }
    }
  }
  keep_trying=(load_count < LOAD_FILES_BATCH && keep_running);
  if (keep_trying) {
    auto iterator_normal=this->grid_setup()->get_iterator_full(viewport_current_state);
    while (keep_trying && load_count < LOAD_FILES_BATCH && keep_running) {
      auto grid_index=iterator_normal->get_next();
      keep_trying=(!grid_index->invalid());
      if (keep_trying) {
        if (this->_check_bounds(grid_index.get()) && grid_setup->square_has_data(grid_index.get())) {
          auto load_successful=this->_load_square(viewport_current_state,
                                                  grid_index.get(),
                                                  zoom_index_lower_limit,
                                                  load_all, grid_setup);
          if (load_successful) { load_count++; }
        }
      }
    }
  }
  // uncomment for something fairly useful for seeing how files load
  // MSG("================================================================================");
  // for (auto zoom_index=this->_zoom_index_length-1; zoom_index >= 0L; zoom_index--) {
  //   MSG("Zoom: " << zoom_index);
  //   for (INT64 j=0L; j < grid_h; j++) {
  //     for (INT64 i=0L; i < grid_w; i++) {
  //       std::cout << this->squares[i][j]->image_array[zoom_index]->is_loaded;
  //     }
  //     std::cout << " === ";
  //     for (INT64 i=0L; i < grid_w; i++) {
  //       std::cout << this->_check_load(viewport_current_state,
  //                                      zoom_index,i,j,zoom_index_lower_limit,load_all);
  //     }
  //     std::cout << "\n";
  //   }
  // }
}

GridPixelSize ImageGrid::image_max_pixel_size() const {
  return this->_image_max_size;
}

ImageGridSquare* ImageGrid::squares(const GridIndex& grid_index) const {
  auto i=grid_index.i();
  auto j=grid_index.j();
  return this->_squares[i][j].get();
}

ImageGridSquare* ImageGrid::squares(const GridIndex* grid_index) const {
  auto i=grid_index->i();
  auto j=grid_index->j();
  return this->_squares[i][j].get();
}

void ImageGrid::setup_grid_cache(GridSetup* const grid_setup) {
  // no read_grid_info(...) called for now
  auto grid_w=this->grid_image_size().w();
  auto grid_h=this->grid_image_size().h();
  this->_read_grid_info_setup_squares(grid_setup);
  // loop over the whole grid
  for (INT64 i=0L; i < grid_w; i++) {
    for (INT64 j=0L; j < grid_h; j++) {
      // load the file into the data structure
      auto grid_index=GridIndex(i,j);
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
      for (auto k=this->_zoom_index_length-1; k >= 0L; k--) {
        this->_squares[i][j]->image_array[k]->unload_square();
      }
    }
  }

}

GridSetup* ImageGrid::grid_setup() const {
  return this->_grid_setup;
}

const GridImageSize ImageGrid::grid_image_size() const {
  return this->_grid_setup->grid_image_size();
}
