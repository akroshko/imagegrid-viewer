/**
 * Implementation for classes to setup the grid to be viewed.
 */
#include "../common.hpp"
#include "../utility.hpp"
#include "gridsetup.hpp"
#include "../datatypes/coordinates.hpp"
#include "../c_io_net/fileload.hpp"
#include "../c_misc/argument_parse.hpp"
#include "../viewport_current_state.hpp"
// C++ headers
#include <iostream>
#include <string>
#include <vector>

GridSetupStatus GridSetup::status() const {
  return this->_status;
}

GridImageSize GridSetup::grid_image_size() const {
  return this->_grid_image_size;
}

bool GridSetup::setup_cache() const {
  return this->_setup_cache;
}

bool GridSetup::use_cache() const {
  return this->_use_cache;
}

GridImageSize GridSetup::grid_size() const {
  return this->_grid_image_size;
}

INT64 GridSetup::_grid_index(INT64 i, INT64 j) const {
  return j*this->grid_size().w()+i;
}

INT64 GridSetup::_grid_index(const GridIndex& grid_index) const {
  auto i=grid_index.i();
  auto j=grid_index.j();
  return this->_grid_index(i,j);
}

INT64 GridSetup::_grid_index(const GridIndex* grid_index) const {
  auto i=grid_index->i();
  auto j=grid_index->j();
  return this->_grid_index(i,j);
}

bool GridSetup::square_has_data(const GridIndex& grid_index) const {
  return this->_existing[grid_index];
}

bool GridSetup::square_has_data(const GridIndex* grid_index) const {
  return this->_existing[*grid_index];
}

SubGridImageSize GridSetup::sub_size(const GridIndex& grid_index) const {
  return SubGridImageSize(this->_sub_size[grid_index]);
}

bool GridSetup::subgrid_has_data(const GridIndex& grid_index, const SubGridIndex& subgrid_index) const {
  auto str=this->filename(grid_index,subgrid_index);
  return (check_valid_filename(str));
}

std::string GridSetup::filename(const GridIndex& grid_index, const SubGridIndex& subgrid_index) const {
  return this->_file_data(grid_index,subgrid_index);
};

void GridSetup::_post_setup() {
  this->_grid_index_values=std::make_unique<GridIndex[]>(this->grid_size().w()*this->grid_size().h());
  // set up the grid objects to be resturned by iterators
  for (INT64 j=0; j < this->grid_size().h(); j++) {
    for (INT64 i=0; i < this->grid_size().w(); i++) {
      this->_grid_index_values[j*this->grid_size().w()+i]=GridIndex(i,j);
    }
  }
  this->_subgrid_index_values=std::make_unique<std::unique_ptr<SubGridIndex[]>[]>(this->grid_size().w()*this->grid_size().h());
  // TODO: could actually use my new iterator here
  for (INT64 j=0; j < this->grid_size().h(); j++) {
    for (INT64 i=0; i < this->grid_size().w(); i++) {
      auto grid_i=j*this->grid_size().w()+i;
      auto grid_index=GridIndex(i,j);
      this->_subgrid_index_values[grid_i]=std::make_unique<SubGridIndex[]>(this->_sub_size[grid_index].w()*this->_sub_size[grid_index].h());
      // get size for each subgrid
      for (INT64 sj=0; sj<this->_sub_size[grid_index].h(); sj++) {
        for (INT64 si=0; si<this->_sub_size[grid_index].w(); si++) {
          auto subgrid_i=sj*this->_sub_size[grid_index].w()+si;
          this->_subgrid_index_values[grid_i][subgrid_i]=SubGridIndex(si,sj);
        }
      }
    }
  }
}

GridSetupFromCommandLine::GridSetupFromCommandLine(int argc, char* const* argv) {
  INT64 wimage, himage;

  if (!parse_standard_arguments(argc, argv, wimage, himage,
                                this->_setup_cache, this->_use_cache,
                                this->_path_value, this->_filenames, this->_text_filename)) {
    MSG_LOCAL("Error parsing arguments");
    std::cout << HELP_STRING << std::endl;
    this->_status=GridSetupStatus::load_error;
    return;
  }
  if (this->_text_filename.length() != 0) {
    INT64 max_i,max_j;
    if (!load_image_grid_from_text(this->_text_filename,
                                   this->_read_data,
                                   max_i,max_j)) {
      // TODO: right now error message is in the above function
      this->_status=GridSetupStatus::load_error;
      return;
    };
    wimage=max_i+1;
    himage=max_j+1;
    this->_grid_image_size=GridImageSize(wimage,himage);
    // initial allocation
    this->_existing.init(this->_grid_image_size);
    this->_sub_size.init(this->_grid_image_size);
    for (INT64 j=0; j < himage; j++) {
      for (INT64 i=0; i < wimage; i++) {
        auto grid_index=GridIndex(i,j);
        this->_existing.set(grid_index,false);
        this->_sub_size.set(grid_index,SubGridImageSize(1,1));
      }
    }
    // iterate to get subgrid width and height and existing
    for (std::list<GridSetupFile>::iterator it = this->_read_data.begin(); it !=this->_read_data.end(); ++it) {
      auto i=it->grid_i;
      auto j=it->grid_j;
      auto sub_i=it->subgrid_i;
      auto sub_j=it->subgrid_j;
      auto grid_index=GridIndex(i,j);
      auto sub_w_current=this->_sub_size[grid_index].w();
      auto sub_h_current=this->_sub_size[grid_index].h();
      if ((sub_i+1) > sub_w_current) {
        this->_sub_size.set(grid_index,SubGridImageSize(sub_i+1,
                                                        this->_sub_size[grid_index].h()));
      }
      if ((sub_j+1) > sub_h_current) {
        this->_sub_size.set(grid_index,SubGridImageSize(this->_sub_size[grid_index].w(),
                                                        sub_j+1));
      }
      this->_existing.set(grid_index,true);
    }
    // setup the new data structure
    this->_file_data.init_layer_1(this->_grid_image_size);
    for (INT64 j=0; j < himage; j++) {
      for (INT64 i=0; i < wimage; i++) {
        auto grid_index=GridIndex(i,j);
        auto sub_w=this->_sub_size[grid_index].w();
        auto sub_h=this->_sub_size[grid_index].h();
        this->_file_data.init_layer_2(grid_index,this->_sub_size[grid_index]);
        // initialize to empty
        for (INT64 sub_j=0; sub_j < sub_h; sub_j++) {
          for (INT64 sub_i=0; sub_i < sub_w; sub_i++) {
            auto subgrid_index=SubGridIndex(sub_i,sub_j);
            this->_file_data.set(grid_index,subgrid_index,"");
          }
        }
      }
    }
    // now load from the deque to the data structure
    while (!this->_read_data.empty()) {
      auto data=this->_read_data.back();
      auto i=data.grid_i;
      auto j=data.grid_j;
      auto sub_i=data.subgrid_i;
      auto sub_j=data.subgrid_j;
      auto grid_index=GridIndex(i,j);
      auto subgrid_index=SubGridIndex(sub_i,sub_j);
      this->_file_data.set(grid_index,subgrid_index,std::string(data.filename));
      this->_read_data.pop_back();
    }
  } else {
    this->_grid_image_size=GridImageSize(wimage,himage);
    ////////////////////////////////////////////////////////////////////////////////
    // to here
    auto grid_size=wimage*himage;
    // load images numbered in directory if this is appropriate
    if (this->_path_value.length() != 0) {
      this->_filenames=load_numbered_images(this->_path_value);
    }
    if (grid_size != (INT64)this->_filenames.size()) {
      this->_status=GridSetupStatus::load_error;
      ERROR_LOCAL("Number of filenames " << this->_filenames.size() << " does not match grid size " << grid_size);
      return;
    }
    // this type of input will not hvae subgrids
    // TODO: this can probably be refactored into above
    this->_grid_image_size=GridImageSize(wimage,himage);
    this->_existing.init(this->_grid_image_size);
    this->_sub_size.init(this->_grid_image_size);
    this->_file_data.init_layer_1(this->_grid_image_size);
    for (INT64 k=0;k<wimage*himage;k++) {
      INT64 i=k%wimage;
      INT64 j=k/wimage;
      auto grid_index=GridIndex(i,j);
      this->_sub_size[grid_index]=SubGridImageSize(1,1);
      this->_file_data.init_layer_2(grid_index,this->_sub_size[grid_index]);
      auto subgrid_index=SubGridIndex(0,0);
      this->_file_data.set(grid_index,subgrid_index,"");
    }
    for (INT64 k=0;k<(INT64)this->_filenames.size();k++) {
      INT64 i=k%wimage;
      INT64 j=k/wimage;
      auto grid_index=GridIndex(i,j);
      auto subgrid_index=SubGridIndex(0,0);
      this->_file_data.set(grid_index,subgrid_index,this->_filenames[k]);
      this->_existing.set(grid_index,true);
    }
  }
  // TODO: call this a little more automatically
  GridSetup::_post_setup();
  if (this->_status != GridSetupStatus::load_error) {
    this->_status=GridSetupStatus::loaded;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Basic iterators
////////////////////////////////////////////////////////////////////////////////

ImageGridBasicIterator::ImageGridBasicIterator(GridSetup* grid_setup) {
  this->_grid_setup=grid_setup;
}

const GridIndex* ImageGridBasicIterator::begin() const {
  return &this->_grid_setup->_grid_index_values[0];
}

const GridIndex* ImageGridBasicIterator::end() const {
  // TODO: don't like indexing out beyond end of array
  return &this->_grid_setup->_grid_index_values[this->_grid_setup->grid_size().w()*this->_grid_setup->grid_size().h()];
}

ImageSubGridBasicIterator::ImageSubGridBasicIterator(GridSetup* grid_setup, const GridIndex& grid_index) {
  this->_grid_setup=grid_setup;
  this->_grid_index=grid_index;
}

const SubGridIndex* ImageSubGridBasicIterator::begin() const {
  auto grid_i=this->_grid_index.j()*this->_grid_setup->grid_size().w()+this->_grid_index.i();
  return &this->_grid_setup->_subgrid_index_values[grid_i][0];
}

const SubGridIndex* ImageSubGridBasicIterator::end() const {
  // TODO: don't like indexing out beyond end of array
  auto grid_i=this->_grid_index.j()*this->_grid_setup->grid_size().w()+this->_grid_index.i();
  auto subgrid_i_end=this->_grid_setup->_sub_size[this->_grid_index].w()*this->_grid_setup->_sub_size[this->_grid_index].h();
  return &this->_grid_setup->_subgrid_index_values[grid_i][subgrid_i_end];
}

////////////////////////////////////////////////////////////////////////////////
// Full iterators
////////////////////////////////////////////////////////////////////////////////

GridIndexPointerProxy::GridIndexPointerProxy(const ImageGridFromViewportIterator* parent_iterator,
                                             INT64 index_value) {
  this->_parent_iterator=parent_iterator;
  this->_index_value=index_value;
}

GridIndex GridIndexPointerProxy::operator*() {
  // TODO: handle end case?
  return this->_parent_iterator->_index_values[this->_index_value];
}

GridIndexPointerProxy GridIndexPointerProxy::operator++() {
  return GridIndexPointerProxy(this->_parent_iterator,
                               this->_index_value++);
}

bool GridIndexPointerProxy::operator!=(GridIndexPointerProxy& compare) {
  return this->_index_value != compare._index_value;
}

ImageGridFromViewportFullIterator::ImageGridFromViewportFullIterator(GridSetup* grid_setup,
                                                                     const ViewPortCurrentState& viewport_current_state) {
  // TODO: this is copied from an older iterator class to test the concept
  //       make a bit better
  auto w_image_grid=grid_setup->grid_image_size().w();
  auto h_image_grid=grid_setup->grid_image_size().h();
  this->_w=w_image_grid;
  this->_h=h_image_grid;
  auto current_grid_x=viewport_current_state.current_grid_coordinate().x();
  auto current_grid_y=viewport_current_state.current_grid_coordinate().y();
  // TODO need a good iterator class for this type of work
  // load the one we are looking at
  // do the center
  // pretend we are in the corner of the grid if we are outside the grid
  auto adjusted_grid_x=(INT64)floor(current_grid_x);
  auto adjusted_grid_y=(INT64)floor(current_grid_y);
  if (current_grid_x > this->_w) {
    adjusted_grid_x=this->_w;
  } else if (current_grid_x < 0) {
    adjusted_grid_x=0;
  }
  if (current_grid_y > this->_h) {
    adjusted_grid_y=this->_h;
  } else if (current_grid_y < 0) {
    adjusted_grid_y=0;
  }
  auto i_initial=adjusted_grid_x;
  auto j_initial=adjusted_grid_y;
  this->_index_values.push_back(GridIndex(i_initial,j_initial));
  // load near the viewport one first then work way out
  for (INT64 r=1L; r <= (std::max(w_image_grid,h_image_grid)); r++) {
    // start at top left corner, go to top right corner
    for (INT64 i=adjusted_grid_x-r; i <= adjusted_grid_x+r; i++) {
      auto j=adjusted_grid_y-r;
      this->_index_values.push_back(GridIndex(i,j));
    }
    // go to bottom right corner
    for (INT64 j=adjusted_grid_y-r; j <= adjusted_grid_y+r; j++) {
      auto i=adjusted_grid_x+r;
      this->_index_values.push_back(GridIndex(i,j));
    }
    // go to bottom left corner
    for (INT64 i=adjusted_grid_x+r; i >= adjusted_grid_x-r; i--) {
      auto j=adjusted_grid_y+r;
      this->_index_values.push_back(GridIndex(i,j));
    }
    // go to top right corner
    for (INT64 j=adjusted_grid_y+r; j >= adjusted_grid_y-r; j--) {
      auto i=adjusted_grid_x-r;
      this->_index_values.push_back(GridIndex(i,j));
    }
  }
}

const GridIndexPointerProxy ImageGridFromViewportFullIterator::begin() const {
  auto grid_index_pointer_proxy=GridIndexPointerProxy(dynamic_cast<const ImageGridFromViewportIterator*>(this),
                                                      0);
  return grid_index_pointer_proxy;
}

const GridIndexPointerProxy ImageGridFromViewportFullIterator::end() const {
  auto grid_index_pointer_proxy=GridIndexPointerProxy(dynamic_cast<const ImageGridFromViewportIterator*>(this),
                                                      this->_index_values.size());
  return grid_index_pointer_proxy;
}

ImageGridFromViewportVisibleIterator::ImageGridFromViewportVisibleIterator(GridSetup* grid_setup,
                                                                           const ViewPortCurrentState& viewport_current_state) {
  // TODO: this is copied from an older iterator class to test the concept
  //       make a bit better
  auto w_image_grid=grid_setup->grid_image_size().w();
  auto h_image_grid=grid_setup->grid_image_size().h();
  this->_w=w_image_grid;
  this->_h=h_image_grid;
  auto current_grid_x=viewport_current_state.current_grid_coordinate().x();
  auto current_grid_y=viewport_current_state.current_grid_coordinate().y();
  auto leftmost_visible=ViewPortTransferState::find_leftmost_visible(viewport_current_state);
  auto rightmost_visible=ViewPortTransferState::find_rightmost_visible(viewport_current_state);
  auto topmost_visible=ViewPortTransferState::find_topmost_visible(viewport_current_state);
  auto bottommost_visible=ViewPortTransferState::find_bottommost_visible(viewport_current_state);
  auto visible_imin=floor(leftmost_visible);
  auto visible_imax=floor(rightmost_visible);
  auto visible_jmin=floor(topmost_visible);
  auto visible_jmax=floor(bottommost_visible);
  auto center_i=(INT64)floor(current_grid_x);
  auto center_j=(INT64)floor(current_grid_y);
  auto r_i=std::max(std::abs(center_i-visible_imin),
                    std::abs(visible_imax-center_i));
  auto r_j=std::max(std::abs(center_j-visible_jmin),
                    std::abs(visible_jmax-center_j));
  auto r_visible=std::max(std::abs(r_i),std::abs(r_j));
  // check visible layer first
  for (INT64 r=0L; r <= r_visible; r++) {
    for (INT64 i=center_i-r; i <= center_i+r; i++) {
      auto j=center_j-r;
      if (i >= 0 && i < w_image_grid && j >= 0 && j < h_image_grid) {
        this->_index_values.push_back(GridIndex(i,j));
      }
    }
    for (INT64 j=center_j-r; j <= center_j+r; j++) {
      auto i=center_i+r;
      if (i >= 0 && i < w_image_grid && j >= 0 && j < h_image_grid) {
        this->_index_values.push_back(GridIndex(i,j));
      }
    }
    for (INT64 i=center_i+r; i >= center_i-r; i--) {
      auto j=center_j+r;
      if (i >= 0 && i < w_image_grid && j >= 0 && j < h_image_grid) {
        this->_index_values.push_back(GridIndex(i,j));
      }
    }
    for (INT64 j=center_j+r; j >= center_j-r; j--) {
      auto i=center_i-r;
      if (i >= 0 && i < w_image_grid && j >= 0 && j < h_image_grid) {
        this->_index_values.push_back(GridIndex(i,j));
      }
    }
  }
}

const GridIndexPointerProxy ImageGridFromViewportVisibleIterator::begin() const {
  auto grid_index_pointer_proxy=GridIndexPointerProxy(dynamic_cast<const ImageGridFromViewportIterator*>(this),
                                                      0);
  return grid_index_pointer_proxy;
}

const GridIndexPointerProxy ImageGridFromViewportVisibleIterator::end() const {
  auto grid_index_pointer_proxy=GridIndexPointerProxy(dynamic_cast<const ImageGridFromViewportIterator*>(this),
                                                      this->_index_values.size());
  return grid_index_pointer_proxy;
}
