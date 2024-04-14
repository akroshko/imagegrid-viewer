#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../src/common.hpp"
#include "../src/utility.hpp"
#include "../src/datatypes/coordinates.hpp"
#include "../src/datatypes/containers.hpp"
#include "../src/c_io_net/fileload.hpp"
#include "../src/imagegrid/imagegrid_load_file_data.hpp"
#include "../src/c_misc/buffer_manip.hpp"
// C++ headers
#include <memory>

// entered manually as a basic test for the whole thing
const unsigned char TEST_IMAGE[80]={
  211,80,233,255,
  244,225,249,255,
  228,174,241,255,
  206,25,231,255,
  230,180,242,255,
  225,162,240,255,
  224,159,239,255,
  220,140,237,255,
  217,130,237,255,
  218,133,237,255,
  221,146,238,255,
  215,117,235,255,
  220,139,237,255,
  218,132,237,255,
  222,150,239,255,
  235,198,245,255,
  218,132,237,255,
  218,128,236,255,
  230,180,242,255,
  237,204,246,255};

// entered manually as a basic test for the whole thing
const unsigned char TEST_IMAGE_BUFFER_EXPAND_2[320]={
  //
  211,80,233,255,
  211,80,233,255,
  244,225,249,255,
  244,225,249,255,
  228,174,241,255,
  228,174,241,255,
  206,25,231,255,
  206,25,231,255,
  230,180,242,255,
  230,180,242,255,
  //
  211,80,233,255,
  211,80,233,255,
  244,225,249,255,
  244,225,249,255,
  228,174,241,255,
  228,174,241,255,
  206,25,231,255,
  206,25,231,255,
  230,180,242,255,
  230,180,242,255,
  //
  225,162,240,255,
  225,162,240,255,
  224,159,239,255,
  224,159,239,255,
  220,140,237,255,
  220,140,237,255,
  217,130,237,255,
  217,130,237,255,
  218,133,237,255,
  218,133,237,255,
  //
  225,162,240,255,
  225,162,240,255,
  224,159,239,255,
  224,159,239,255,
  220,140,237,255,
  220,140,237,255,
  217,130,237,255,
  217,130,237,255,
  218,133,237,255,
  218,133,237,255,
  //
  221,146,238,255,
  221,146,238,255,
  215,117,235,255,
  215,117,235,255,
  220,139,237,255,
  220,139,237,255,
  218,132,237,255,
  218,132,237,255,
  222,150,239,255,
  222,150,239,255,
  //
  221,146,238,255,
  221,146,238,255,
  215,117,235,255,
  215,117,235,255,
  220,139,237,255,
  220,139,237,255,
  218,132,237,255,
  218,132,237,255,
  222,150,239,255,
  222,150,239,255,
  //
  235,198,245,255,
  235,198,245,255,
  218,132,237,255,
  218,132,237,255,
  218,128,236,255,
  218,128,236,255,
  230,180,242,255,
  230,180,242,255,
  237,204,246,255,
  237,204,246,255,
  //
  235,198,245,255,
  235,198,245,255,
  218,132,237,255,
  218,132,237,255,
  218,128,236,255,
  218,128,236,255,
  230,180,242,255,
  230,180,242,255,
  237,204,246,255,
  237,204,246,255};

TEST_CASE("Does doctest work?") {
  CHECK(2+2 == 4);
  CHECK(2+3 != 4);
}

TEST_CASE("Do the utility functions work?") {
  CHECK(pad(32,0)==32);
  CHECK(pad(32,2)==32);
  CHECK(pad(37,2)==38);
  CHECK(reduce_and_pad(32,0)==32);
  CHECK(reduce_and_pad(32,2)==16);
  CHECK(reduce_and_pad(35,2)==18);
  // checking valid filnames does not check for existence currently
  // but simply avoids empty strings
  CHECK(check_valid_filename("/bin/bash"));
  CHECK(check_valid_filename("") == false);
  CHECK(shift_left_signed(0x10,0) == 0x10);
  CHECK(shift_left_signed(0x10,1) == 0x20);
  CHECK(shift_left_signed(0x10,-1) == 0x08);
  CHECK(shift_right_signed(0x10,0) == 0x10);
  CHECK(shift_right_signed(0x10,1) == 0x08);
  CHECK(shift_right_signed(0x10,-1) == 0x20);
  CHECK(ceil_minus_one(1.5) == 1.0);
  CHECK(ceil_minus_one(1.0) == 1.0);
  CHECK(ceil_minus_one(-1.0) == -1.0);
  CHECK(ceil_minus_one(-1.5) == -2.0);
}

TEST_CASE("Does basic functionality of coordinates and containers work?") {
  ////////////////////////////////////////////////////////////////////////////////
  // Coordinates
  ////////////////////////////////////////////////////////////////////////////////
  // GridImageSize
  auto grid_image_size_1=GridImageSize(1,1);
  auto grid_image_size_2=GridImageSize(3,4);
  GridImageSize grid_image_size_3=static_cast<GridImageSize>(grid_image_size_1+grid_image_size_2);
  CHECK(grid_image_size_3.w() == 4);
  CHECK(grid_image_size_3.h() == 5);
  // GridCoordinateSize
  auto grid_coordinate_size_1=GridCoordinateSize(1.125,2.875);
  auto grid_coordinate_size_2=GridCoordinateSize(2.625,7.5);
  GridCoordinateSize grid_coordinate_size_3=static_cast<GridCoordinateSize>(grid_coordinate_size_1+grid_coordinate_size_2);
  CHECK(grid_coordinate_size_3.w() == 3.75);
  CHECK(grid_coordinate_size_3.h() == 10.375);
  // GridCoordinate
  auto grid_coordinate_1=GridCoordinate(3.25,9.125);
  auto grid_coordinate_2=GridCoordinate(4.625,2.875);
  GridCoordinate grid_coordinate_3=static_cast<GridCoordinate>(grid_coordinate_1+grid_coordinate_2);
  CHECK(grid_coordinate_3.x() == 7.875);
  CHECK(grid_coordinate_3.y() == 12.0);
  // GridIndex
  auto grid_index_1=GridIndex(2,3);
  auto grid_index_2=GridIndex(5,6);
  GridIndex grid_index_3=static_cast<GridIndex>(grid_index_1+grid_index_2);
  CHECK(grid_index_3.i() == 7);
  CHECK(grid_index_3.j() == 9);
  // SubGridImageSize
  auto subgrid_index_1=SubGridIndex(3,4);
  auto subgrid_index_2=SubGridIndex(5,6);
  SubGridIndex subgrid_index_3=static_cast<SubGridIndex>(subgrid_index_1+subgrid_index_2);
  CHECK(subgrid_index_3.i() == 8);
  CHECK(subgrid_index_3.j() == 10);
  // GridPixelSize
  auto grid_pixel_size_1=GridPixelSize(300,400);
  auto grid_pixel_size_2=GridPixelSize(900,1000);
  GridPixelSize grid_pixel_size_3=static_cast<GridPixelSize>(grid_pixel_size_1+grid_pixel_size_2);
  CHECK(grid_pixel_size_3.w() == 1200);
  CHECK(grid_pixel_size_3.h() == 1400);
  // ImagePixelCoordinate
  auto image_pixel_coordinate_1=ImagePixelCoordinate(345,890);
  auto image_pixel_coordinate_2=ImagePixelCoordinate(670,234);
  ImagePixelCoordinate image_pixel_coordinate_3=static_cast<ImagePixelCoordinate>(image_pixel_coordinate_1+image_pixel_coordinate_2);
  CHECK(image_pixel_coordinate_3.x() == 1015);
  CHECK(image_pixel_coordinate_3.y() == 1124);
  // BufferTileSize
  auto buffer_tile_size_1=BufferTileSize(1,2);
  auto buffer_tile_size_2=BufferTileSize(3,4);
  BufferTileSize buffer_tile_size_3=static_cast<BufferTileSize>(buffer_tile_size_1+buffer_tile_size_2);
  CHECK(buffer_tile_size_3.w() == 4);
  CHECK(buffer_tile_size_3.h() == 6);
  // BufferTileIndex
  auto buffer_tile_index_1=BufferTileIndex(3,9);
  auto buffer_tile_index_2=BufferTileIndex(4,2);
  BufferTileIndex buffer_tile_index_3=static_cast<BufferTileIndex>(buffer_tile_index_1+buffer_tile_index_2);
  CHECK(buffer_tile_index_3.i() == 7);
  CHECK(buffer_tile_index_3.j() == 11);
  // BufferPixelSize
  auto buffer_pixel_size_1=BufferPixelSize(492,509);
  auto buffer_pixel_size_2=BufferPixelSize(623,456);
  BufferPixelSize buffer_pixel_size_3=static_cast<BufferPixelSize>(buffer_pixel_size_1+buffer_pixel_size_2);
  CHECK(buffer_pixel_size_3.w() == 1115);
  CHECK(buffer_pixel_size_3.h() == 965);
  // BufferPixelCoordinate
  auto buffer_pixel_coordinate_1=BufferPixelCoordinate(230,823);
  auto buffer_pixel_coordinate_2=BufferPixelCoordinate(345,870);
  BufferPixelCoordinate buffer_pixel_coordinate_3=static_cast<BufferPixelCoordinate>(buffer_pixel_coordinate_1+buffer_pixel_coordinate_2);
  CHECK(buffer_pixel_coordinate_3.x() == 575);
  CHECK(buffer_pixel_coordinate_3.y() == 1693);
  ////////////////////////////////////////////////////////////////////////////////
  // containers
  ////////////////////////////////////////////////////////////////////////////////
  // StaticArray
  StaticArray<std::unique_ptr<INT64>> static_array;
  static_array.init(9);
  static_array.set(1,std::make_unique<INT64>(23));
  static_array.set(3,std::make_unique<INT64>(31));
  CHECK(*static_array[1] == 23);
  CHECK(*static_array[3] == 31);
  ////////////////////////////////////////////////////////////////////////////////
  // StaticGrid
  StaticGrid<INT64> static_grid;
  auto static_grid_size=GridImageSize(3,4);
  static_grid.init(static_grid_size);
  auto static_grid_index=GridIndex(2,3);
  static_grid.set(static_grid_index,72);
  CHECK(static_grid[static_grid_index] == 72);
  // StaticGrid<std::unique_ptr<T>>
  StaticGrid<std::unique_ptr<INT64>> static_grid_ptr;
  static_grid_ptr.init(static_grid_size);
  static_grid_ptr.set(static_grid_index,std::make_unique<INT64>(73));
  CHECK(*static_grid_ptr[static_grid_index] == 73);
  ////////////////////////////////////////////////////////////////////////////////
  // StaticGridTwoLayer
  StaticGridTwoLayer<INT64> static_grid_two_layer;
  auto static_grid_layer_1_size=GridImageSize(4,5);
  static_grid_two_layer.init_layer_1(static_grid_layer_1_size);
  auto static_grid_index_1=GridIndex(1,2);
  auto static_grid_layer_2_size_1=SubGridImageSize(2,3);
  static_grid_two_layer.init_layer_2(static_grid_index_1,static_grid_layer_2_size_1);
  auto static_grid_index_2=GridIndex(3,2);
  auto static_grid_layer_2_size_2=SubGridImageSize(2,1);
  static_grid_two_layer.init_layer_2(static_grid_index_2,static_grid_layer_2_size_2);
  auto static_grid_layer_2_index_1=SubGridIndex(1,2);
  static_grid_two_layer.set(static_grid_index_1,static_grid_layer_2_index_1,65);
  auto static_grid_layer_2_index_2=SubGridIndex(0,0);
  static_grid_two_layer.set(static_grid_index_2,static_grid_layer_2_index_2,85);
  CHECK(static_grid_two_layer(static_grid_index_1,static_grid_layer_2_index_1) == 65);
  CHECK(static_grid_two_layer(static_grid_index_2,static_grid_layer_2_index_2) == 85);
}

// it would be nice to combine repeated code in PNG and TIFF tests,
// but I don't want to deal with macro within macro errors or false
// positives from an incorrectly coded function right now
TEST_CASE("Do PNG images load correctly?") {
  const INT64 test_image_wpixel=5;
  const INT64 test_image_hpixel=4;
  // meant to be run from project root
  std::string filename="./tests/test_small.png";
  // temporary scratch buffer so doesn't need to be initialized
  INT64 row_temp_buffer[128];
  auto subgrid_index=SubGridIndex(0,0);
  auto subgrid_size=SubGridImageSize(1,1);
  // create the data transfer function
  LoadFileDataTransfer load_file_data_transfer;
  load_file_data_transfer.sub_size=SubGridImageSize(subgrid_size);
  load_file_data_transfer.original_rgba_wpixel.init(subgrid_size);
  load_file_data_transfer.original_rgba_wpixel.set(subgrid_index,test_image_wpixel);
  load_file_data_transfer.original_rgba_hpixel.init(subgrid_size);
  load_file_data_transfer.original_rgba_hpixel.set(subgrid_index,test_image_hpixel);
  auto load_file_zoom_level_data=std::make_shared<LoadFileZoomLevelData>();
  load_file_zoom_level_data->filename=filename;
  load_file_zoom_level_data->rgba_wpixel.init(subgrid_size);
  load_file_zoom_level_data->rgba_hpixel.init(subgrid_size);
  load_file_zoom_level_data->max_sub_wpixel=test_image_wpixel;
  load_file_zoom_level_data->max_sub_hpixel=test_image_hpixel;
  load_file_zoom_level_data->rgba_data.init(subgrid_size);
  load_file_zoom_level_data->zoom_out_shift=0;
  load_file_data_transfer.data_transfer.emplace_back(load_file_zoom_level_data);
  load_png_as_rgba(filename,subgrid_index,load_file_data_transfer,row_temp_buffer);
  // MSG_LOCAL("==============================");
  // check individual pixels
  for (INT64 j=0; j < test_image_hpixel; j++) {
    for (INT64 i=0; i < test_image_wpixel; i++) {
      auto rgb_index=j*test_image_wpixel+i;
      auto pixel=load_file_data_transfer.data_transfer[0]->rgba_data[subgrid_index][rgb_index];
      // uncomment to see individual pixels
      // MSG_LOCAL("i: " << i << " j: " << j << " " <<
      //           ((pixel & 0x000000FF))       << " " <<
      //           ((pixel & 0x0000FF00) >> 8)  << " " <<
      //           ((pixel & 0x00FF0000) >> 16) << " " <<
      //           ((pixel & 0xFF000000) >> 24));
      CHECK(((pixel & 0x000000FF))       == TEST_IMAGE[rgb_index*4]);
      CHECK(((pixel & 0x0000FF00) >> 8)  == TEST_IMAGE[rgb_index*4+1]);
      CHECK(((pixel & 0x00FF0000) >> 16) == TEST_IMAGE[rgb_index*4+2]);
      CHECK(((pixel & 0xFF000000) >> 24) == TEST_IMAGE[rgb_index*4+3]);
    }
  }
  // MSG_LOCAL("==============================");
}

// it would be nice to combine repeated code in PNG and TIFF tests,
// but I don't want to deal with macro within macro errors or false
// positives from an incorrectly coded function right now
TEST_CASE("Do TIFF images load correctly?") {
  const INT64 test_image_wpixel=5;
  const INT64 test_image_hpixel=4;
  // meant to be run from project root
  std::string filename="./tests/test_small.tif";
  // temporary scratch buffer so doesn't need to be initialized
  INT64 row_temp_buffer[128];
  auto subgrid_index=SubGridIndex(0,0);
  auto subgrid_size=SubGridImageSize(1,1);
  // create the data transfer function
  LoadFileDataTransfer load_file_data_transfer;
  load_file_data_transfer.sub_size=SubGridImageSize(subgrid_size);
  load_file_data_transfer.original_rgba_wpixel.init(subgrid_size);
  load_file_data_transfer.original_rgba_wpixel.set(subgrid_index,test_image_wpixel);
  load_file_data_transfer.original_rgba_hpixel.init(subgrid_size);
  load_file_data_transfer.original_rgba_hpixel.set(subgrid_index,test_image_hpixel);
  auto load_file_zoom_level_data=std::make_shared<LoadFileZoomLevelData>();
  load_file_zoom_level_data->filename=filename;
  load_file_zoom_level_data->rgba_wpixel.init(subgrid_size);
  load_file_zoom_level_data->rgba_hpixel.init(subgrid_size);
  load_file_zoom_level_data->max_sub_wpixel=test_image_wpixel;
  load_file_zoom_level_data->max_sub_hpixel=test_image_hpixel;
  load_file_zoom_level_data->rgba_data.init(subgrid_size);
  load_file_zoom_level_data->zoom_out_shift=0;
  load_file_data_transfer.data_transfer.emplace_back(load_file_zoom_level_data);
  load_tiff_as_rgba(filename,subgrid_index,load_file_data_transfer,row_temp_buffer);
  // MSG_LOCAL("==============================");
  // check individual pixels
  for (INT64 j=0; j < test_image_hpixel; j++) {
    for (INT64 i=0; i < test_image_wpixel; i++) {
      auto rgb_index=j*test_image_wpixel+i;
      auto pixel=load_file_data_transfer.data_transfer[0]->rgba_data[subgrid_index][rgb_index];
      // uncomment to see individual pixels
      // MSG_LOCAL("i: " << i << " j: " << j << " " <<
      //           ((pixel & 0x000000FF))       << " " <<
      //           ((pixel & 0x0000FF00) >> 8)  << " " <<
      //           ((pixel & 0x00FF0000) >> 16) << " " <<
      //           ((pixel & 0xFF000000) >> 24));
      CHECK(((pixel & 0x000000FF))       == TEST_IMAGE[rgb_index*4]);
      CHECK(((pixel & 0x0000FF00) >> 8)  == TEST_IMAGE[rgb_index*4+1]);
      CHECK(((pixel & 0x00FF0000) >> 16) == TEST_IMAGE[rgb_index*4+2]);
      CHECK(((pixel & 0xFF000000) >> 24) == TEST_IMAGE[rgb_index*4+3]);
    }
  }
  // MSG_LOCAL("==============================");
}

// it would be nice to combine repeated code in PNG and TIFF tests,
// but I don't want to deal with macro within macro errors or false
// positives from an incorrectly coded function right now
std::shared_ptr<LoadFileZoomLevelData> load_rgb_buffer_from_tiff(std::string filename) {
  const INT64 test_image_wpixel=5;
  const INT64 test_image_hpixel=4;
  // meant to be run from project root
  // temporary scratch buffer so doesn't need to be initialized
  INT64 row_temp_buffer[128];
  auto subgrid_index=SubGridIndex(0,0);
  auto subgrid_size=SubGridImageSize(1,1);
  // create the data transfer function
  LoadFileDataTransfer load_file_data_transfer;
  load_file_data_transfer.sub_size=SubGridImageSize(subgrid_size);
  load_file_data_transfer.original_rgba_wpixel.init(subgrid_size);
  load_file_data_transfer.original_rgba_wpixel.set(subgrid_index,test_image_wpixel);
  load_file_data_transfer.original_rgba_hpixel.init(subgrid_size);
  load_file_data_transfer.original_rgba_hpixel.set(subgrid_index,test_image_hpixel);
  auto load_file_zoom_level_data=std::make_shared<LoadFileZoomLevelData>();
  load_file_zoom_level_data->filename=filename;
  load_file_zoom_level_data->rgba_wpixel.init(subgrid_size);
  load_file_zoom_level_data->rgba_hpixel.init(subgrid_size);
  load_file_zoom_level_data->max_sub_wpixel=test_image_wpixel;
  load_file_zoom_level_data->max_sub_hpixel=test_image_hpixel;
  load_file_zoom_level_data->rgba_data.init(subgrid_size);
  load_file_zoom_level_data->zoom_out_shift=0;
  load_file_data_transfer.data_transfer.emplace_back(load_file_zoom_level_data);
  load_tiff_as_rgba(filename,subgrid_index,load_file_data_transfer,row_temp_buffer);
  return std::move(load_file_data_transfer.data_transfer[0]);
}

// it would be nice to combine repeated code in PNG and TIFF tests,
// but I don't want to deal with macro within macro errors or false
// positives from an incorrectly coded function right now
TEST_CASE("Does our function for loading images to test work?") {
  auto subgrid_index=SubGridIndex(0,0);
  auto load_data=load_rgb_buffer_from_tiff("./tests/test_small.tif");
  auto test_image_wpixel=load_data->rgba_wpixel[subgrid_index];
  auto test_image_hpixel=load_data->rgba_hpixel[subgrid_index];
  // MSG_LOCAL("==============================");
  // check individual pixels
  for (INT64 j=0; j < test_image_hpixel; j++) {
    for (INT64 i=0; i < test_image_wpixel; i++) {
      auto rgb_index=j*test_image_wpixel+i;
      auto pixel=load_data->rgba_data[subgrid_index][rgb_index];
      // uncomment to see individual pixels
      // MSG_LOCAL("i: " << i << " j: " << j << " " <<
      //           ((pixel & 0x000000FF))       << " " <<
      //           ((pixel & 0x0000FF00) >> 8)  << " " <<
      //           ((pixel & 0x00FF0000) >> 16) << " " <<
      //           ((pixel & 0xFF000000) >> 24));
      CHECK(((pixel & 0x000000FF))       == TEST_IMAGE[rgb_index*4]);
      CHECK(((pixel & 0x0000FF00) >> 8)  == TEST_IMAGE[rgb_index*4+1]);
      CHECK(((pixel & 0x00FF0000) >> 16) == TEST_IMAGE[rgb_index*4+2]);
      CHECK(((pixel & 0xFF000000) >> 24) == TEST_IMAGE[rgb_index*4+3]);
    }
  }
  // MSG_LOCAL("==============================");
}

TEST_CASE("Does buffer copy work correctly?") {
  auto subgrid_index=SubGridIndex(0,0);
  auto load_data=load_rgb_buffer_from_tiff("./tests/test_small.tif");
  auto source_size=BufferPixelSize(load_data->rgba_wpixel[subgrid_index],
                                   load_data->rgba_hpixel[subgrid_index]);
  auto origin_start=BufferPixelCoordinate(0,0);
  auto dest_size=BufferPixelSize(2*load_data->rgba_wpixel[subgrid_index],
                                 2*load_data->rgba_hpixel[subgrid_index]);
  auto dest_buffer=std::make_unique<PIXEL_RGBA>(2*load_data->rgba_wpixel[subgrid_index]*
                                                2*load_data->rgba_hpixel[subgrid_index]);
  buffer_copy_expand_generic(load_data->rgba_data[subgrid_index],
                             source_size,
                             origin_start,
                             source_size,
                             dest_buffer.get(),
                             dest_size,
                             dest_size,
                             origin_start,
                             1);
  for (INT64 j=0; j < 2*load_data->rgba_wpixel[subgrid_index]; j++) {
    for (INT64 i=0; i < 2*load_data->rgba_hpixel[subgrid_index]; i++) {
      auto rgb_index=j*load_data->rgba_wpixel[subgrid_index]+i;
      auto dest_pixel=dest_buffer.get()[rgb_index];
      CHECK(((dest_pixel & 0x000000FF))       == TEST_IMAGE_BUFFER_EXPAND_2[rgb_index*4]);
      CHECK(((dest_pixel & 0x0000FF00) >> 8)  == TEST_IMAGE_BUFFER_EXPAND_2[rgb_index*4+1]);
      CHECK(((dest_pixel & 0x00FF0000) >> 16) == TEST_IMAGE_BUFFER_EXPAND_2[rgb_index*4+2]);
      CHECK(((dest_pixel & 0xFF000000) >> 24) == TEST_IMAGE_BUFFER_EXPAND_2[rgb_index*4+3]);
    }
  }
}
