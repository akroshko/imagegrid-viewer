/**
 * Implementation of the main classes representing the grid.  Includes
 * both loaded images and (zoomed) textures.
 */
// local headers
#include "../common.hpp"
#include "../utility.hpp"
#include "gridsetup.hpp"
#include "imagegrid.hpp"
#include "../iterators.hpp"
// C compatible headers
#include "../c_io_net/fileload.hpp"
// C++ headers
#include <array>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>
// C headers
#include <cmath>

ImageGridSquareZoomLevel::ImageGridSquareZoomLevel(ImageGridSquare* parent_square,
                                                   INT_T zoom_out_value) {
  this->_parent_square=parent_square;
  this->_max_subgrid_wpixel=this->_parent_square->_max_subgrid_wpixel/zoom_out_value;
  this->_max_subgrid_hpixel=this->_parent_square->_max_subgrid_hpixel/zoom_out_value;
  this->_zoom_out_value=zoom_out_value;
  this->_rgb_wpixel=std::make_unique<std::unique_ptr<size_t[]>[]>(this->subgrid_width());
  this->_rgb_hpixel=std::make_unique<std::unique_ptr<size_t[]>[]>(this->subgrid_width());
  this->_rgb_xpixel_origin=std::make_unique<std::unique_ptr<INT_T[]>[]>(this->subgrid_width());
  this->_rgb_ypixel_origin=std::make_unique<std::unique_ptr<INT_T[]>[]>(this->subgrid_width());
  this->_rgb_data=std::make_unique<std::unique_ptr<unsigned char*[]>[]>(this->subgrid_width());
  for (INT_T i=0L; i < this->subgrid_width(); i++) {
    this->_rgb_wpixel[i]=std::make_unique<size_t[]>(this->subgrid_height());
    this->_rgb_hpixel[i]=std::make_unique<size_t[]>(this->subgrid_height());
    this->_rgb_xpixel_origin[i]=std::make_unique<INT_T[]>(this->subgrid_height());
    this->_rgb_ypixel_origin[i]=std::make_unique<INT_T[]>(this->subgrid_height());
    this->_rgb_data[i]=std::make_unique<unsigned char*[]>(this->subgrid_height());
  }
}

ImageGridSquareZoomLevel::~ImageGridSquareZoomLevel() {
  this->unload_square();
}

bool ImageGridSquareZoomLevel::load_square(ImageGridSquare* grid_square,
                                           bool use_cache,
                                           std::vector<ImageGridSquareZoomLevel*> dest_squares) {
  bool load_successful=true;
  // iterate over square data
  std::vector<std::pair<ImageGridSquareZoomLevel* const,std::shared_ptr<LoadSquareData>>> data_pairs;
  std::vector<std::shared_ptr<LoadSquareData>> data_read;
  for (auto& dest_square : dest_squares) {
    data_pairs.emplace_back(std::pair<ImageGridSquareZoomLevel* const,
                            std::shared_ptr<LoadSquareData>>(dest_square,std::make_shared<LoadSquareData>()));
    data_pairs.back().second->zoom_out_value=dest_square->zoom_out_value();
    data_pairs.back().second->subgrid_width=dest_square->subgrid_width();
    data_pairs.back().second->subgrid_height=dest_square->subgrid_height();
    data_pairs.back().second->max_subgrid_wpixel=dest_square->max_subgrid_hpixel();
    data_pairs.back().second->max_subgrid_hpixel=dest_square->max_subgrid_wpixel();
    data_read.emplace_back(data_pairs.back().second);
  }
  // block until things load
  // only block things actually being loaded

  // need a more automatic solution to initialization like this
  for (auto& data_read_temp : data_read) {
    for (INT_T sub_i=0; sub_i < data_read_temp->subgrid_width; sub_i++) {
      for (INT_T sub_j=0; sub_j < data_read_temp->subgrid_height; sub_j++) {
        auto subgrid_index=SubGridIndex(sub_i,sub_j);
        if (grid_square->grid_setup()->subgrid_has_data(grid_square->_grid_index,
                                                        subgrid_index)) {
          CURRENT_SUBGRID_T current_subgrid(sub_i,sub_j);
          data_read_temp->rgb_data[current_subgrid]=nullptr;
          data_read_temp->rgb_wpixel[current_subgrid]=INT_MIN;
          data_read_temp->rgb_hpixel[current_subgrid]=INT_MIN;
        }
      }
    }
  }
  for (INT_T sub_i=0; sub_i < grid_square->subgrid_width(); sub_i++) {
    for (INT_T sub_j=0; sub_j < grid_square->subgrid_height(); sub_j++) {
      auto subgrid_index=SubGridIndex(sub_i,sub_j);
      if (grid_square->grid_setup()->subgrid_has_data(grid_square->_grid_index,
                                                      subgrid_index)) {
        auto filename=grid_square->grid_setup()->get_filename(grid_square->_grid_index,subgrid_index);
        std::string cached_filename;
        if (use_cache) {
          cached_filename=create_cache_filename(filename);
        } else {
          // TODO: something better for invalid cached filename
          cached_filename="";
        }
        CURRENT_SUBGRID_T current_subgrid(sub_i,sub_j);
        auto load_successful_temp=load_data_as_rgb(filename,
                                                   cached_filename,
                                                   current_subgrid,
                                                   data_read);
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
    for (auto& data_pair : data_pairs) {
      std::lock_guard<std::mutex> guard(data_pair.first->load_mutex);
      for (INT_T sub_i=0; sub_i < grid_square->subgrid_width(); sub_i++) {
        for (INT_T sub_j=0; sub_j < grid_square->subgrid_height(); sub_j++) {
          auto subgrid_index=SubGridIndex(sub_i,sub_j);
          if (grid_square->grid_setup()->subgrid_has_data(grid_square->_grid_index,
                                                          subgrid_index)) {
            auto origin_x=sub_i*(data_pair.first->_max_subgrid_wpixel);
            auto origin_y=sub_j*(data_pair.first->_max_subgrid_hpixel);
            CURRENT_SUBGRID_T current_subgrid(sub_i,sub_j);
            data_pair.first->_rgb_wpixel[sub_i][sub_j]=data_pair.second->rgb_wpixel[current_subgrid];
            data_pair.first->_rgb_hpixel[sub_i][sub_j]=data_pair.second->rgb_hpixel[current_subgrid];
            data_pair.first->_rgb_xpixel_origin[sub_i][sub_j]=origin_x;
            data_pair.first->_rgb_ypixel_origin[sub_i][sub_j]=origin_y;
            data_pair.first->_rgb_data[sub_i][sub_j]=data_pair.second->rgb_data[current_subgrid];
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
    for (INT_T i=0; i < this->_parent_square->subgrid_width(); i++) {
      for (INT_T j=0; j < this->_parent_square->subgrid_height(); j++) {
        if (this->_rgb_data[i][j]) {
          delete[] this->_rgb_data[i][j];
          this->_rgb_data[i][j]=nullptr;
        }
      }
    }
  }
}

INT_T ImageGridSquareZoomLevel::zoom_out_value() const {
  return this->_zoom_out_value;
}

size_t ImageGridSquareZoomLevel::rgb_wpixel(SubGridIndex& subgrid_index) const {
  return this->_rgb_wpixel[subgrid_index.i_subgrid()][subgrid_index.j_subgrid()];
}

size_t ImageGridSquareZoomLevel::rgb_hpixel(SubGridIndex& subgrid_index) const {
  return this->_rgb_hpixel[subgrid_index.i_subgrid()][subgrid_index.j_subgrid()];
}

INT_T ImageGridSquareZoomLevel::rgb_xpixel_origin(SubGridIndex& subgrid_index) const {
  return this->_rgb_xpixel_origin[subgrid_index.i_subgrid()][subgrid_index.j_subgrid()];
}

INT_T ImageGridSquareZoomLevel::rgb_ypixel_origin(SubGridIndex& subgrid_index) const {
  return this->_rgb_ypixel_origin[subgrid_index.i_subgrid()][subgrid_index.j_subgrid()];
}

unsigned char* ImageGridSquareZoomLevel::get_rgb_data(SubGridIndex& subgrid_index) const {
  return this->_rgb_data[subgrid_index.i_subgrid()][subgrid_index.j_subgrid()];
}

INT_T ImageGridSquareZoomLevel::subgrid_width() const {
  return this->_parent_square->subgrid_width();
}

INT_T ImageGridSquareZoomLevel::subgrid_height() const {
  return this->_parent_square->subgrid_height();
}

INT_T ImageGridSquareZoomLevel::max_subgrid_wpixel() const {
  return this->_max_subgrid_wpixel;
}
INT_T ImageGridSquareZoomLevel::max_subgrid_hpixel() const {
  return this->_max_subgrid_hpixel;
}

ImageGridSquare::ImageGridSquare(GridSetup* grid_setup,
                                 ImageGrid* parent_grid,
                                 const GridIndex& grid_index) {
  this->_grid_setup=grid_setup;
  this->_parent_grid=parent_grid;
  this->_grid_index=GridIndex(grid_index);
  this->_read_data();
}

void ImageGridSquare::_read_data() {
  std::vector<INT_T> column_max;
  std::vector<INT_T> row_max;

  // TODO: right now the code works best for sparse/empty if these default to one
  INT_T max_subgrid_pixel_width=1;
  INT_T max_subgrid_pixel_height=1;

  INT_T subgrid_width=this->_grid_setup->subgrid_w(this->_grid_index);
  INT_T subgrid_height=this->_grid_setup->subgrid_h(this->_grid_index);

  for (INT_T sub_i=0; sub_i < subgrid_width; sub_i++) {
    for (INT_T sub_j=0; sub_j < subgrid_height; sub_j++) {
      INT_T image_wpixel;
      INT_T image_hpixel;
      // auto filename=square_data[subgrid_kv_pair.first];
      auto subgrid_index=SubGridIndex(sub_i,sub_j);
      if (this->grid_setup()->subgrid_has_data(this->_grid_index,
                                               subgrid_index)) {
        auto filename=this->_grid_setup->get_filename(this->_grid_index,
                                                      subgrid_index);
        read_data(filename,
                  image_wpixel,
                  image_hpixel);
        if (image_wpixel > max_subgrid_pixel_width) {
          max_subgrid_pixel_width=image_wpixel;
        }
        if (image_hpixel > max_subgrid_pixel_height) {
          max_subgrid_pixel_height=image_hpixel;
        }
      }
    }
  }
  this->_image_wpixel=max_subgrid_pixel_width*subgrid_width;
  this->_image_hpixel=max_subgrid_pixel_height*subgrid_height;
  this->_max_subgrid_wpixel=max_subgrid_pixel_width;
  this->_max_subgrid_hpixel=max_subgrid_pixel_height;
}

void ImageGrid::_read_grid_info_setup_squares(GridSetup* const grid_setup) {
  // delayed allocation for the squares
  auto grid_wimage=grid_setup->grid_image_size().wimage();
  auto grid_himage=grid_setup->grid_image_size().himage();
  this->_squares=std::make_unique<std::unique_ptr<std::unique_ptr<ImageGridSquare>[]>[]>(grid_wimage);
  for (INT_T i=0L; i < grid_setup->grid_image_size().wimage(); i++) {
    this->_squares[i]=std::make_unique<std::unique_ptr<ImageGridSquare>[]>(grid_himage);
  }
  this->_image_max_size=GridPixelSize(0,0);
  // not ready for an iterator until full sparsity is implemented and tested
  INT_T new_wpixel=INT_MIN;
  INT_T new_hpixel=INT_MIN;
  for (INT_T i=0; i<grid_wimage;i++) {
    for (INT_T j=0; j<grid_himage;j++) {
      this->_squares[i][j]=std::make_unique<ImageGridSquare>(grid_setup,this,GridIndex(i,j));
      // set the RGB of the surface
      auto rgb_wpixel=this->_squares[i][j]->_image_wpixel;
      auto rgb_hpixel=this->_squares[i][j]->_image_hpixel;
      // TODO: encapsulate calculation of max pixels while aligning
      if ((INT_T)rgb_wpixel > new_wpixel) {
        new_wpixel=(INT_T)(rgb_wpixel+(TEXTURE_ALIGNMENT - (rgb_wpixel % TEXTURE_ALIGNMENT)));
      }
      if ((INT_T)rgb_hpixel > new_hpixel) {
        new_hpixel=(INT_T)rgb_hpixel;
      }
    }
  }
  this->_image_max_size=GridPixelSize(new_wpixel,new_hpixel);;
  auto image_max_size_wpixel=this->_image_max_size.wpixel();
  auto image_max_size_hpixel=this->_image_max_size.hpixel();

  // find how many zoom_step to get whole image grid as a 3x3 grid of original size
  // TODO: revise description of why this works
  auto max_scale=(INT_T)ceil((FLOAT_T)(fmax(image_max_size_wpixel,image_max_size_hpixel))/(FLOAT_T)MAX_MIN_SCALED_IMAGE_SIZE);
  this->_zoom_index_length=(INT_T)ceil(log2(max_scale));
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
  for (INT_T i=0L; i < this->grid_image_size().wimage(); i++) {
    for (INT_T j=0L; j < this->grid_image_size().himage(); j++) {
      this->_squares[i][j]->image_array=std::make_unique<std::unique_ptr<ImageGridSquareZoomLevel>[]>(this->_zoom_index_length);
      INT_T zoom_out_value=1;
      for (auto k=0L; k < this->_zoom_index_length; k++) {
        this->_squares[i][j]->image_array[k]=std::make_unique<ImageGridSquareZoomLevel>(this->_squares[i][j].get(),
                                                                                       zoom_out_value);
        zoom_out_value*=zoom_step;
      }
    }
  }
}

GridSetup* ImageGridSquare::grid_setup() const {
  return this->_grid_setup;
}

INT_T ImageGridSquare::subgrid_width() {
  return this->grid_setup()->subgrid_w(this->_grid_index);
}

INT_T ImageGridSquare::subgrid_height() {
  return this->grid_setup()->subgrid_h(this->_grid_index);
}

ImageGrid* ImageGridSquare::parent_grid() const {
  return this->_parent_grid;
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

INT_T ImageGrid::zoom_index_length() const {
  return this->_zoom_index_length;
}

bool ImageGrid::_check_bounds(const GridIndex& grid_index) {
  auto i=grid_index.i_grid();
  auto j=grid_index.j_grid();
  auto return_value=(i >= 0 && i < this->grid_image_size().wimage() && j >= 0 && j < this->grid_image_size().himage());
  return return_value;
}

bool ImageGrid::_check_load(const ViewPortCurrentState& viewport_current_state,
                            INT_T zoom_index,
                            const GridIndex& grid_index,
                            INT_T zoom_index_lower_limit,
                            INT_T load_all) {
  auto i=grid_index.i_grid();
  auto j=grid_index.j_grid();
  auto current_grid_x=viewport_current_state.current_grid_coordinate().xgrid();
  auto current_grid_y=viewport_current_state.current_grid_coordinate().ygrid();
  auto screen_width=viewport_current_state.screen_size().hpixel();
  auto screen_height=viewport_current_state.screen_size().wpixel();
  // TODO: don't like this so need to keep working on transferring calculations
  auto viewport_current_state_new=ViewPortCurrentState(GridCoordinate(current_grid_x,current_grid_y),
                                                       GridPixelSize(this->_image_max_size.wpixel(), this->_image_max_size.hpixel()),
                                                       ViewPortTransferState::find_zoom_upper(zoom_index),
                                                       ViewportPixelSize(screen_width,screen_height));
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
                             const GridIndex& grid_index,
                             INT_T zoom_index_lower_limit,
                             INT_T load_all, const GridSetup* const grid_setup) {
  // always load if top level
  bool tried_load=false;
  bool never_false=true;
  if (this->_check_bounds(grid_index)) {
    std::vector<INT_T> zoom_index_list;
    for (INT_T zoom_index=this->_zoom_index_length-1; zoom_index >= 0L; zoom_index--) {
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
      for (auto & zoom_index : zoom_index_list) {
        dest_squares.push_back(this->squares(grid_index)->image_array[zoom_index].get());
      }
      tried_load=true;
      auto load_successful_temp=ImageGridSquareZoomLevel::load_square(this->squares(grid_index),
                                                                      grid_setup->use_cache(),
                                                                      dest_squares);
      if (!load_successful_temp) {
        never_false=false;
      }
    }
  }
  return (tried_load && never_false);
}

void ImageGrid::_write_cache(const GridIndex& grid_index) {
  auto w_sub=this->_grid_setup->subgrid_w(grid_index);
  auto h_sub=this->_grid_setup->subgrid_h(grid_index);
  for (INT_T sub_i=0; sub_i < w_sub; sub_i++) {
    for (INT_T sub_j=0; sub_j < h_sub; sub_j++) {
      auto subgrid_index=SubGridIndex(sub_i,sub_j);
      bool loaded_cache_size=false;
      for (INT_T k=0L; k<this->_zoom_index_length; k++) {
        if (loaded_cache_size) {
          break;
        }
        auto dest_square=this->squares(grid_index)->image_array[k].get();
        // find current size
        auto wpixel=dest_square->rgb_wpixel(subgrid_index);
        auto hpixel=dest_square->rgb_hpixel(subgrid_index);
        // TODO: check size of filename_new here
        auto filename=this->grid_setup()->get_filename(grid_index,subgrid_index);
        auto filename_new=create_cache_filename(filename);
        MSG("Trying to writing cache filename: " <<
            filename_new << " for " << filename << " at zoom index " << k <<
            " i: " << grid_index.i_grid() << " j: " << grid_index.j_grid() <<
            " sub_i: " << sub_i << " sub_j: " << sub_j);
        if (w_sub*wpixel < CACHE_MAX_PIXEL_SIZE && h_sub*hpixel < CACHE_MAX_PIXEL_SIZE) {
          loaded_cache_size=write_png(filename_new, wpixel, hpixel, dest_square->_rgb_data[sub_i][sub_j]);
          MSG("Cache tried with return: " << loaded_cache_size);
          if (loaded_cache_size) {
            MSG("Cached worked with w:" << wpixel << " h:" << hpixel);
          }
        }
      }
    }
  }
}

void ImageGrid::load_grid(const GridSetup* const grid_setup, std::atomic<bool> &keep_running) {
  auto keep_trying=true;
  // get information on viewport
  auto viewport_current_state=this->_viewport_current_state_imagegrid_update->GetGridValues();
  // find the grid extents and choose things that should be loaded/unloaded
  // different things for what should be loaded/unloaded
  // load in reverse order of size
  auto current_zoom_index=ViewPortTransferState::find_zoom_index_bounded(viewport_current_state.zoom(),0.0,this->_zoom_index_length-1);
  auto grid_w=this->grid_image_size().wimage();
  auto grid_h=this->grid_image_size().himage();
  auto load_all=false;
  auto zoom_index_lower_limit=current_zoom_index-1;
  // unload first
  for (auto zoom_index=this->_zoom_index_length-1; zoom_index >= 0L; zoom_index--) {
    for (INT_T i=0L; i < grid_w; i++) {
      for (INT_T j=0L; j < grid_h; j++) {
        if (!keep_running) {
          keep_trying=false;
        }
        auto grid_index=GridIndex(i,j);
        if (!this->_check_load(viewport_current_state,
                               zoom_index, grid_index, zoom_index_lower_limit,
                               load_all)) {
          this->_squares[i][j]->image_array[zoom_index]->unload_square();
          // always try and unload rest, except top level
        }
      }
    }
  }
  INT_T i,j;
  // files actually loaded
  INT_T load_count=0;
  auto iterator_visible=ImageGridIteratorVisible(this->grid_image_size().wimage(),this->grid_image_size().himage(),
                                                 viewport_current_state);
  // TODO need a good iterator class for this type of work load what
  // we are looking at if things are not loaded
  while (keep_trying) {
    keep_trying=iterator_visible.get_next(i,j);
    if (!keep_running) { break; }
    if (keep_trying) {
      auto grid_index=GridIndex(i,j);
      if (this->_check_bounds(grid_index) && grid_setup->square_has_data(grid_index)) {

        auto load_successful=this->_load_square(viewport_current_state,
                                                grid_index,
                                                zoom_index_lower_limit,
                                                load_all, grid_setup);
        if (load_successful) { load_count++; }
        if (load_count >= LOAD_FILES_BATCH) { keep_trying=false; }
      }
    }
  }
  auto iterator_normal=ImageGridIteratorFull(this->grid_image_size().wimage(),
                                             this->grid_image_size().himage(),
                                             viewport_current_state);
  // TODO need a good iterator class for this type of work
  // load the one we are looking at
  keep_trying=(!(load_count >= LOAD_FILES_BATCH || !keep_running));

  while (keep_trying) {
    keep_trying=iterator_normal.get_next(i,j);
    if (!keep_running) { break; }
    if (keep_trying) {
      auto grid_index=GridIndex(i,j);
      if (this->_check_bounds(grid_index) && grid_setup->square_has_data(grid_index)) {
        auto load_successful=this->_load_square(viewport_current_state,
                                                grid_index,
                                                zoom_index_lower_limit,
                                                load_all, grid_setup);
        if (load_successful) { load_count++; }
        if (load_count >= LOAD_FILES_BATCH) { keep_trying=false; }
      }
    }
  }
  // uncomment for something fairly useful for seeing how files load
  // MSG("================================================================================");
  // for (auto zoom_index=this->_zoom_index_length-1; zoom_index >= 0L; zoom_index--) {
  //   MSG("Zoom: " << zoom_index);
  //   for (INT_T j=0L; j < grid_h; j++) {
  //     for (INT_T i=0L; i < grid_w; i++) {
  //       std::cout << this->squares[i][j]->image_array[zoom_index]->is_loaded;
  //     }
  //     std::cout << " === ";
  //     for (INT_T i=0L; i < grid_w; i++) {
  //       std::cout << this->_check_load(viewport_current_state,
  //                                      zoom_index,i,j,zoom_index_lower_limit,load_all);
  //     }
  //     std::cout << "\n";
  //   }
  // }
}

GridPixelSize ImageGrid::get_image_max_pixel_size() const {
  return this->_image_max_size;
}

ImageGridSquare* ImageGrid::squares(const GridIndex& grid_index) const {
  auto i=grid_index.i_grid();
  auto j=grid_index.j_grid();
  return this->_squares[i][j].get();
}

void ImageGrid::setup_grid_cache(GridSetup* const grid_setup) {
  // no read_grid_info(...) called for now
  auto grid_w=this->grid_image_size().wimage();
  auto grid_h=this->grid_image_size().himage();
  this->_read_grid_info_setup_squares(grid_setup);
  // loop over the whole grid
  for (INT_T i=0L; i < grid_w; i++) {
    for (INT_T j=0L; j < grid_h; j++) {
      // load the file into the data structure
      auto grid_index=GridIndex(i,j);
      this->_load_square(ViewPortCurrentState(GridCoordinate(0.0,0.0),
                                              GridPixelSize(0,0),
                                              0.0,
                                              ViewportPixelSize(0,0)),
                         grid_index,
                         0L,true,grid_setup);
      // TODO: eventually cache this out as tiles that fit in 128x128 and 512x512
      auto file_data=grid_setup->file_data();
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

const GridImageSize ImageGrid::grid_image_size() {
  return this->_grid_setup->grid_image_size();
}
