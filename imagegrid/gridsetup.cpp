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

const FILE_DATA_T& GridSetup::file_data() const {
  return this->_file_data;
}

bool GridSetup::do_cache() const {
  return this->_do_cache;
}

bool GridSetup::use_cache() const {
  return this->_use_cache;
}

INT_T GridSetup::grid_w() const {
  return _grid_image_size.himage();
}

INT_T GridSetup::grid_h() const {
  return _grid_image_size.wimage();
}

bool GridSetup::square_has_data(const GridIndex& grid_index) const {
  // TODO: check bounds
  return _existing[grid_index.i_grid()][grid_index.j_grid()];
}

INT_T GridSetup::subgrid_w(const GridIndex& grid_index) const {
  // TODO: this has to be changed
  std::pair<INT_T,INT_T> grid_pair(grid_index.i_grid(),grid_index.j_grid());
  return this->_subgrid_width[grid_index.i_grid()][grid_index.j_grid()];
}

INT_T GridSetup::subgrid_h(const GridIndex& grid_index) const {
  // TODO: this has to be changed
  std::pair<INT_T,INT_T> grid_pair(grid_index.i_grid(),grid_index.j_grid());
  return this->_subgrid_height[grid_index.i_grid()][grid_index.j_grid()];
}

bool GridSetup::subgrid_has_data(const GridIndex& grid_index, const SubGridIndex& subgrid_index) const {
  std::pair<INT_T,INT_T> grid_pair(grid_index.i_grid(),grid_index.j_grid());
  std::pair<INT_T,INT_T> subgrid_pair(subgrid_index.i_subgrid(),subgrid_index.j_subgrid());
  auto file_data=this->_file_data;
  return (file_data[grid_pair].count(subgrid_pair) > 0);
}

std::string GridSetup::get_filename(const GridIndex& grid_index, const SubGridIndex& subgrid_index) const {
  std::pair<INT_T,INT_T> grid_pair(grid_index.i_grid(),grid_index.j_grid());
  std::pair<INT_T,INT_T> subgrid_pair(subgrid_index.i_subgrid(),subgrid_index.j_subgrid());
  auto file_data=this->_file_data;
  return file_data[grid_pair][subgrid_pair];
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
    load_image_grid_from_text(this->_text_filename,this->_file_data,
                              max_i,max_j);
    wimage=max_i+1;
    himage=max_j+1;
    this->_grid_image_size=GridImageSize(wimage,himage);
    this->_successful=true;
    // TODO: this code should be updated
    //       better way to initialize or data structures
    // initialize whether grid exists
    this->_existing.reserve(wimage);
    for (INT_T i=0; i<wimage; i++) {
      std::vector<bool> temp_row;
      temp_row.reserve(himage);
      for (INT_T j=0; j<himage; j++) {
        // TODO: this is likely inefficient but only exists in
        // one-time setup code
        std::pair<INT_T,INT_T> grid_pair(i,j);
        temp_row.push_back((this->_file_data[grid_pair].size() > 0));
      }
      this->_existing.push_back(temp_row);
    }
    // initialize subgrid width and height
    this->_subgrid_height.reserve(wimage);
    this->_subgrid_width.reserve(wimage);
    for (INT_T i=0; i<wimage; i++) {
      std::vector<INT_T> temp_row_subgrid_height;
      temp_row_subgrid_height.reserve(himage);
      std::vector<INT_T> temp_row_subgrid_width;
      temp_row_subgrid_width.reserve(himage);
      for (INT_T j=0; j<himage; j++) {
        // TODO: this is likely inefficient but only exists in
        // one-time setup code
        std::pair<INT_T,INT_T> grid_pair(i,j);
        if (this->_existing[i][j]) {
          INT_T subgrid_width=1;
          INT_T subgrid_height=1;
          auto file_data=this->_file_data[grid_pair];
          for (auto& subgrid_kv_pair : file_data) {
            auto sub_i=subgrid_kv_pair.first.first;
            auto sub_j=subgrid_kv_pair.first.second;
            if ((sub_i+1) > subgrid_width) {
              subgrid_width=(sub_i+1);
            }
            if ((sub_j+1) > subgrid_height) {
              subgrid_height=(sub_j+1);
            }
          }
          // TODO: this is temporary, to bridge existing code to new code
          temp_row_subgrid_width.push_back(subgrid_width);
          temp_row_subgrid_height.push_back(subgrid_height);
        } else {
          // TODO: this is what is assumed for empty squares
          temp_row_subgrid_height.push_back(1);
          temp_row_subgrid_width.push_back(1);
        }
      }
      this->_subgrid_width.push_back(temp_row_subgrid_width);
      this->_subgrid_height.push_back(temp_row_subgrid_height);
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
      auto current_subgrid=CURRENT_SUBGRID_T(0,0);
      this->_file_data[current_grid][current_subgrid]=this->_filenames[i];
    }
    this->_successful=true;
  }
};
