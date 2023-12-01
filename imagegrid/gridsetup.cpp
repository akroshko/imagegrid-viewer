/**
 * Implementation for classes to setup the grid to be viewed.
 */
#include "../common.hpp"
#include "../utility.hpp"
#include "gridsetup.hpp"
#include "../coordinates.hpp"
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

INT64 GridSetup::_sub_index(INT64 sub_i, INT64 sub_j, INT64 sub_w) const {
  return sub_j*sub_w+sub_i;
}

INT64 GridSetup::_sub_index(const SubGridIndex& sub_index, INT64 sub_w) const {
  auto sub_i=sub_index.i();
  auto sub_j=sub_index.j();
  return this->_sub_index(sub_i, sub_j, sub_w);
}

bool GridSetup::square_has_data(const GridIndex& grid_index) const {
  auto local_grid_index=this->_grid_index(grid_index);
  return this->_existing[local_grid_index];
}

bool GridSetup::square_has_data(const GridIndex* grid_index) const {
  auto local_grid_index=this->_grid_index(grid_index);
  return this->_existing[local_grid_index];
}

SubGridImageSize GridSetup::sub_size(const GridIndex& grid_index) const {
  auto local_grid_index=this->_grid_index(grid_index);
  return SubGridImageSize(this->_sub_size[local_grid_index]);
}

bool GridSetup::subgrid_has_data(const GridIndex& grid_index, const SubGridIndex& sub_index) const {
  auto str=this->filename(grid_index,sub_index);
  return (check_valid_filename(str));
}

std::string GridSetup::filename(const GridIndex& grid_index, const SubGridIndex& sub_index) const {
  auto local_grid_index=this->_grid_index(grid_index);
  auto sub_w=this->_sub_size[local_grid_index].w();
  auto local_sub_index=this->_sub_index(sub_index,sub_w);
  return this->_file_data[local_grid_index][local_sub_index];
};

std::unique_ptr<ImageGridIteratorVisible> GridSetup::iterator_visible(const ViewPortCurrentState& viewport_current_state) {
  return std::make_unique<ImageGridIteratorVisible>(this->grid_image_size().w(),
                                                    this->grid_image_size().h(),
                                                    viewport_current_state);
}

std::unique_ptr<ImageGridIteratorFull> GridSetup::iterator_full(const ViewPortCurrentState& viewport_current_state) {
  return std::make_unique<ImageGridIteratorFull>(this->grid_image_size().w(),
                                                 this->grid_image_size().h(),
                                                 viewport_current_state);
}

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
      this->_subgrid_index_values[grid_i]=std::make_unique<SubGridIndex[]>(this->_sub_size[grid_i].w()*this->_sub_size[grid_i].h());
      // get size for each subgrid
      for (INT64 sj=0; sj<this->_sub_size[grid_i].h(); sj++) {
        for (INT64 si=0; si<this->_sub_size[grid_i].w(); si++) {
          auto subgrid_i=sj*this->_sub_size[grid_i].w()+si;
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
    MSG("Error parsing arguments");
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
    this->_existing=std::make_unique<bool[]>(wimage*himage);
    this->_sub_size=std::make_unique<SubGridImageSize[]>(wimage*himage);
    for (INT64 j=0; j < himage; j++) {
      for (INT64 i=0; i < wimage; i++) {
        auto grid_index=this->_grid_index(i, j);
        this->_existing[grid_index]=false;
        this->_sub_size[grid_index]=SubGridImageSize(1,1);
      }
    }
    // iterate to get subgrid width and height and existing
    for (std::list<GridSetupFile>::iterator it = this->_read_data.begin(); it !=this->_read_data.end(); ++it) {
      auto i=it->grid_i;
      auto j=it->grid_j;
      auto sub_i=it->subgrid_i;
      auto sub_j=it->subgrid_j;
      auto grid_index=this->_grid_index(i, j);
      auto sub_w_current=this->_sub_size[grid_index].w();
      auto sub_h_current=this->_sub_size[grid_index].h();
      if ((sub_i+1) > sub_w_current) {
        this->_sub_size[grid_index]=SubGridImageSize(sub_i+1,
                                                     this->_sub_size[grid_index].h());
      }
      if ((sub_j+1) > sub_h_current) {
        this->_sub_size[grid_index]=SubGridImageSize(this->_sub_size[grid_index].w(),
                                                     sub_j+1);
      }
      this->_existing[grid_index]=true;
    }
    // setup the new data structure
    this->_file_data=std::make_unique<std::unique_ptr<std::string[]>[]>(wimage*himage);
    for (INT64 j=0; j < himage; j++) {
      for (INT64 i=0; i < wimage; i++) {
        auto grid_index=this->_grid_index(i, j);
        auto sub_w=this->_sub_size[grid_index].w();
        auto sub_h=this->_sub_size[grid_index].h();
        this->_file_data[grid_index]=std::make_unique<std::string[]>(sub_w*sub_h);
        // initialize to empty
        for (INT64 sub_j=0; sub_j < sub_h; sub_j++) {
          for (INT64 sub_i=0; sub_i < sub_w; sub_i++) {
            auto sub_index=this->_sub_index(sub_i, sub_j, sub_w);
            this->_file_data[grid_index][sub_index]="";
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
      auto grid_index=this->_grid_index(i, j);
      auto sub_w=this->_sub_size[grid_index].w();
      auto sub_index=this->_sub_index(sub_i, sub_j, sub_w);
      // TODO: avoid this copy
      this->_file_data[grid_index][sub_index]=std::string(data.filename);
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
      ERROR("Number of filenames " << this->_filenames.size() << " does not match grid size " << grid_size);
      return;
    }
    // this type of input will not hae subgrids
    // TODO: this can probably be refactored into above
    this->_existing=std::make_unique<bool[]>(wimage*himage);
    this->_sub_size=std::make_unique<SubGridImageSize[]>(wimage*himage);
    this->_file_data=std::make_unique<std::unique_ptr<std::string[]>[]>(wimage*himage);
    this->_grid_image_size=GridImageSize(wimage,himage);
    for (INT64 k=0;k<wimage*himage;k++) {
      INT64 i=k%wimage;
      INT64 j=k/wimage;
      auto grid_index=this->_grid_index(i, j);
      this->_sub_size[grid_index]=SubGridImageSize(1,1);
      this->_file_data[grid_index]=std::make_unique<std::string[]>(1);
      auto sub_index=this->_sub_index(0, 0, 1);
      this->_file_data[grid_index][sub_index]="";
    }
    for (INT64 k=0;k<(INT64)this->_filenames.size();k++) {
      INT64 i=k%wimage;
      INT64 j=k/wimage;
      auto grid_index=this->_grid_index(i, j);
      auto sub_index=this->_sub_index(0, 0, 1);
      this->_file_data[grid_index][sub_index]=this->_filenames[k];
      this->_existing[grid_index]=true;
    }
  }
  // TODO: call this a little more automatically
  GridSetup::_post_setup();
  if (this->_status != GridSetupStatus::load_error) {
    this->_status=GridSetupStatus::loaded;
  }
}

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
  auto subgrid_i_end=this->_grid_setup->_sub_size[grid_i].w()*this->_grid_setup->_sub_size[grid_i].h();
  return &this->_grid_setup->_subgrid_index_values[grid_i][subgrid_i_end];
}
