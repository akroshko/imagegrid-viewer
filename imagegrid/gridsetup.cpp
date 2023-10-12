/**
 * Implementation for classes to setup the grid to be viewed.
 */
#include "../common.hpp"
#include "gridsetup.hpp"
#include "../coordinates.hpp"
#include "../c_io_net/fileload.hpp"
#include "../c_misc/argument_parse.hpp"
// C++ headers
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

const std::string HELP_STRING=
  "Usage: imagegrid-viewer [-c|-d] -w WIDTH -h HEIGHT IMAGES...\n"
  "       imagegrid-viewer [-c|-d] -f TEXT_FILE\n"
  "\n"
  "  -c        create cache\n"
  "  -d        use cache\n"
  "\n"
  "  -w        width of grid in images\n"
  "  -h        height of grid in images\n"
  "\n"
  "  -f        text file with each line in the format:\n"
  "            X_INDEX Y_INDEX X_SUBGRID_INDEX Y_SUBGRID_INDEX FILENAME\n";

bool GridSetup::successful() const {
  return this->_successful;
}

GridImageSize GridSetup::grid_image_size() const {
  return this->_grid_image_size;
}

bool GridSetup::do_cache() const {
  return this->_do_cache;
}

bool GridSetup::use_cache() const {
  return this->_use_cache;
}

INT_T GridSetup::grid_w() const {
  return _grid_image_size.wimage();
}

INT_T GridSetup::grid_h() const {
  return _grid_image_size.himage();
}

INT_T GridSetup::_get_grid_index(INT_T i, INT_T j) const {
  return j*this->grid_w()+i;
}

INT_T GridSetup::_get_grid_index(const GridIndex& grid_index) const {
  auto i=grid_index.i_grid();
  auto j=grid_index.j_grid();
  return j*this->grid_w()+i;
}

INT_T GridSetup::_get_subgrid_index(INT_T sub_i, INT_T sub_j, INT_T sub_w) const {
  return sub_j*sub_w+sub_i;
}

INT_T GridSetup::_get_subgrid_index(const SubGridIndex& subgrid_index, INT_T sub_w) const {
  auto sub_i=subgrid_index.i_subgrid();
  auto sub_j=subgrid_index.j_subgrid();
  return sub_j*sub_w+sub_i;
}

bool GridSetup::square_has_data(const GridIndex& grid_index) const {
  auto local_grid_index=this->_get_grid_index(grid_index);
  return this->_existing[local_grid_index];
}

INT_T GridSetup::subgrid_w(const GridIndex& grid_index) const {
  auto local_grid_index=this->_get_grid_index(grid_index);
  return this->_subgrid_width[local_grid_index];
}

INT_T GridSetup::subgrid_h(const GridIndex& grid_index) const {
  auto local_grid_index=this->_get_grid_index(grid_index);
  return this->_subgrid_height[local_grid_index];
}

bool GridSetup::subgrid_has_data(const GridIndex& grid_index, const SubGridIndex& subgrid_index) const {
  auto str=this->get_filename(grid_index,subgrid_index);
  return (!(str == ""));
}

std::string GridSetup::get_filename(const GridIndex& grid_index, const SubGridIndex& subgrid_index) const {
  auto local_grid_index=this->_get_grid_index(grid_index);
  auto sub_w=this->_subgrid_width[local_grid_index];
  auto local_subgrid_index=this->_get_subgrid_index(subgrid_index,sub_w);
  return this->_new_file_data[local_grid_index][local_subgrid_index];
};

GridSetupFromCommandLine::GridSetupFromCommandLine(int argc, char* const* argv) {
  INT_T wimage, himage;
  parse_standard_arguments(argc, argv, wimage, himage,
                           this->_do_cache, this->_use_cache, this->_successful,
                           this->_path_value, this->_filenames, this->_text_filename);
  if (!this->_successful) {
    MSG("Error parsing arguments");
    std::cout << HELP_STRING << std::endl;
    return;
  }
  if (this->_text_filename.length() != 0) {
    INT_T max_i,max_j;
    load_image_grid_from_text(this->_text_filename,
                              this->_read_data,
                              max_i,max_j);
    wimage=max_i+1;
    himage=max_j+1;
    this->_grid_image_size=GridImageSize(wimage,himage);
    this->_successful=true;
    // initial allocation
    this->_existing=std::make_unique<bool[]>(wimage*himage);
    this->_subgrid_width=std::make_unique<INT_T[]>(wimage*himage);
    this->_subgrid_height=std::make_unique<INT_T[]>(wimage*himage);
    for (INT_T j=0; j < himage; j++) {
      for (INT_T i=0; i < wimage; i++) {
        auto grid_index=this->_get_grid_index(i, j);
        this->_existing[grid_index]=false;
        this->_subgrid_width[grid_index]=1;
        this->_subgrid_height[grid_index]=1;
      }
    }
    // iterate to get subgrid width and height and existing
    for (std::list<GridSetupFile>::iterator it = this->_read_data.begin(); it !=this->_read_data.end(); ++it) {
      auto i=it->grid_i;
      auto j=it->grid_j;
      auto sub_i=it->subgrid_i;
      auto sub_j=it->subgrid_j;
      auto grid_index=this->_get_grid_index(i, j);
      auto sub_w_current=this->_subgrid_width[grid_index];
      auto sub_h_current=this->_subgrid_height[grid_index];
      if ((sub_i+1) > sub_w_current) {
        this->_subgrid_width[grid_index]=(sub_i+1);
      }
      if ((sub_j+1) > sub_h_current) {
        this->_subgrid_height[grid_index]=(sub_j+1);
      }
      this->_existing[grid_index]=true;
    }
    // setup the new data structure
    this->_new_file_data=std::make_unique<std::unique_ptr<std::string[]>[]>(wimage*himage);
    for (INT_T j=0; j < himage; j++) {
      for (INT_T i=0; i < wimage; i++) {
        auto grid_index=this->_get_grid_index(i, j);
        auto sub_w=this->_subgrid_width[grid_index];
        auto sub_h=this->_subgrid_height[grid_index];
        this->_new_file_data[grid_index]=std::make_unique<std::string[]>(sub_w*sub_h);
        // initialize to empty
        for (INT_T sub_j=0; sub_j < sub_h; sub_j++) {
          for (INT_T sub_i=0; sub_i < sub_w; sub_i++) {
            auto subgrid_index=this->_get_subgrid_index(sub_i, sub_j, sub_w);
            this->_new_file_data[grid_index][subgrid_index]="";
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
      auto sub_w=this->_subgrid_width[grid_index];
      auto subgrid_index=this->_get_subgrid_index(sub_i, sub_j, sub_w);
      // TODO: avoid this copy
      this->_new_file_data[grid_index][subgrid_index]=std::string(data.filename);
      this->_read_data.pop_back();
    }
  } else {
    this->_grid_image_size=GridImageSize(wimage,himage);
    ////////////////////////////////////////////////////////////////////////////////
    // to here
    auto grid_size=himage*wimage;
    if (grid_size != this->_filenames.size()) {
      this->_successful=false;
      ERROR("Number of filenames " << this->_filenames.size() << " does not match grid size " << grid_size);
      return;
    }
    // load images numbered in directory if this is appropriate
    if (this->_path_value.length() != 0) {
      this->_filenames=load_numbered_images(this->_path_value);
    }
    for (INT_T i=0;i < this->_filenames.size();i++) {
      INT_T x=i%wimage;
      INT_T y=i/wimage;
      auto current_grid=std::pair<INT_T,INT_T>(x,y);
      // auto current_subgrid=CURRENT_SUBGRID_T(0,0);
      // this->_file_data[current_grid][current_subgrid]=this->_filenames[i];
    }
    this->_successful=true;
  }
};
