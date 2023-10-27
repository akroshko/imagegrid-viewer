/**
 * Implementation for classes to setup the grid to be viewed.
 */
#include "../common.hpp"
#include "../utility.hpp"
#include "gridsetup.hpp"
#include "../coordinates.hpp"
#include "../c_io_net/fileload.hpp"
#include "../c_misc/argument_parse.hpp"
// C++ headers
#include <iostream>
#include <string>
#include <vector>

bool GridSetup::successful() const {
  return this->_successful;
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

INT64 GridSetup::grid_w() const {
  return _grid_image_size.wimage();
}

INT64 GridSetup::grid_h() const {
  return _grid_image_size.himage();
}

INT64 GridSetup::_get_grid_index(INT64 i, INT64 j) const {
  return j*this->grid_w()+i;
}

INT64 GridSetup::_get_grid_index(const GridIndex& grid_index) const {
  auto i=grid_index.i_grid();
  auto j=grid_index.j_grid();
  return j*this->grid_w()+i;
}

INT64 GridSetup::_get_subgrid_index(INT64 sub_i, INT64 sub_j, INT64 sub_w) const {
  return sub_j*sub_w+sub_i;
}

INT64 GridSetup::_get_subgrid_index(const SubGridIndex& subgrid_index, INT64 sub_w) const {
  auto sub_i=subgrid_index.subgrid_i();
  auto sub_j=subgrid_index.subgrid_j();
  return sub_j*sub_w+sub_i;
}

bool GridSetup::square_has_data(const GridIndex& grid_index) const {
  auto local_grid_index=this->_get_grid_index(grid_index);
  return this->_existing[local_grid_index];
}

INT64 GridSetup::subgrid_w(const GridIndex& grid_index) const {
  auto local_grid_index=this->_get_grid_index(grid_index);
  return this->_subgrid_w[local_grid_index];
}

INT64 GridSetup::subgrid_h(const GridIndex& grid_index) const {
  auto local_grid_index=this->_get_grid_index(grid_index);
  return this->_subgrid_h[local_grid_index];
}

bool GridSetup::subgrid_has_data(const GridIndex& grid_index, const SubGridIndex& subgrid_index) const {
  auto str=this->get_filename(grid_index,subgrid_index);
  return (check_valid_filename(str));
}

std::string GridSetup::get_filename(const GridIndex& grid_index, const SubGridIndex& subgrid_index) const {
  auto local_grid_index=this->_get_grid_index(grid_index);
  auto sub_w=this->_subgrid_w[local_grid_index];
  auto local_subgrid_index=this->_get_subgrid_index(subgrid_index,sub_w);
  return this->_file_data[local_grid_index][local_subgrid_index];
};

GridSetupFromCommandLine::GridSetupFromCommandLine(int argc, char* const* argv) {
  INT64 wimage, himage;
  parse_standard_arguments(argc, argv, wimage, himage,
                           this->_setup_cache, this->_use_cache, this->_successful,
                           this->_path_value, this->_filenames, this->_text_filename);
  if (!this->_successful) {
    MSG("Error parsing arguments");
    std::cout << HELP_STRING << std::endl;
    return;
  }
  if (this->_text_filename.length() != 0) {
    INT64 max_i,max_j;
    load_image_grid_from_text(this->_text_filename,
                              this->_read_data,
                              max_i,max_j);
    wimage=max_i+1;
    himage=max_j+1;
    this->_grid_image_size=GridImageSize(wimage,himage);
    this->_successful=true;
    // initial allocation
    this->_existing=std::make_unique<bool[]>(wimage*himage);
    this->_subgrid_w=std::make_unique<INT64[]>(wimage*himage);
    this->_subgrid_h=std::make_unique<INT64[]>(wimage*himage);
    for (INT64 j=0; j < himage; j++) {
      for (INT64 i=0; i < wimage; i++) {
        auto grid_index=this->_get_grid_index(i, j);
        this->_existing[grid_index]=false;
        this->_subgrid_w[grid_index]=1;
        this->_subgrid_h[grid_index]=1;
      }
    }
    // iterate to get subgrid width and height and existing
    for (std::list<GridSetupFile>::iterator it = this->_read_data.begin(); it !=this->_read_data.end(); ++it) {
      auto i=it->grid_i;
      auto j=it->grid_j;
      auto sub_i=it->subgrid_i;
      auto sub_j=it->subgrid_j;
      auto grid_index=this->_get_grid_index(i, j);
      auto sub_w_current=this->_subgrid_w[grid_index];
      auto sub_h_current=this->_subgrid_h[grid_index];
      if ((sub_i+1) > sub_w_current) {
        this->_subgrid_w[grid_index]=(sub_i+1);
      }
      if ((sub_j+1) > sub_h_current) {
        this->_subgrid_h[grid_index]=(sub_j+1);
      }
      this->_existing[grid_index]=true;
    }
    // setup the new data structure
    this->_file_data=std::make_unique<std::unique_ptr<std::string[]>[]>(wimage*himage);
    for (INT64 j=0; j < himage; j++) {
      for (INT64 i=0; i < wimage; i++) {
        auto grid_index=this->_get_grid_index(i, j);
        auto sub_w=this->_subgrid_w[grid_index];
        auto sub_h=this->_subgrid_h[grid_index];
        this->_file_data[grid_index]=std::make_unique<std::string[]>(sub_w*sub_h);
        // initialize to empty
        for (INT64 sub_j=0; sub_j < sub_h; sub_j++) {
          for (INT64 sub_i=0; sub_i < sub_w; sub_i++) {
            auto subgrid_index=this->_get_subgrid_index(sub_i, sub_j, sub_w);
            this->_file_data[grid_index][subgrid_index]="";
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
      auto grid_index=this->_get_grid_index(i, j);
      auto sub_w=this->_subgrid_w[grid_index];
      auto subgrid_index=this->_get_subgrid_index(sub_i, sub_j, sub_w);
      // TODO: avoid this copy
      this->_file_data[grid_index][subgrid_index]=std::string(data.filename);
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
    if (grid_size != this->_filenames.size()) {
      this->_successful=false;
      ERROR("Number of filenames " << this->_filenames.size() << " does not match grid size " << grid_size);
      return;
    }
    // this type of input will not hae subgrids
    // TODO: this can probably be refactored into above
    this->_existing=std::make_unique<bool[]>(wimage*himage);
    this->_subgrid_w=std::make_unique<INT64[]>(wimage*himage);
    this->_subgrid_h=std::make_unique<INT64[]>(wimage*himage);
    this->_file_data=std::make_unique<std::unique_ptr<std::string[]>[]>(wimage*himage);
    this->_grid_image_size=GridImageSize(wimage,himage);
    for (INT64 k=0;k < wimage*himage;k++) {
      INT64 i=k%wimage;
      INT64 j=k/wimage;
      auto grid_index=this->_get_grid_index(i, j);
      this->_subgrid_w[grid_index]=1;
      this->_subgrid_h[grid_index]=1;
      this->_file_data[grid_index]=std::make_unique<std::string[]>(1);
      auto subgrid_index=this->_get_subgrid_index(0, 0, 1);
      this->_file_data[grid_index][subgrid_index]="";
    }
    for (INT64 k=0;k < this->_filenames.size();k++) {
      INT64 i=k%wimage;
      INT64 j=k/wimage;
      auto grid_index=this->_get_grid_index(i, j);
      auto subgrid_index=this->_get_subgrid_index(0, 0, 1);
      this->_file_data[grid_index][subgrid_index]=this->_filenames[k];
      this->_existing[grid_index]=true;
    }
    this->_successful=true;
  }
}
