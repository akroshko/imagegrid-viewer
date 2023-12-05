#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../common.hpp"
#include "../datatypes/coordinates.hpp"
#include "../datatypes/containers.hpp"
// C++ headers
#include <memory>

TEST_CASE("Does doctest work?") {
  CHECK(2+2 == 4);
  CHECK(2+3 != 4);
}

TEST_CASE("Does basic functionality of coordinates and containers work") {
  ////////////////////////////////////////////////////////////////////////////////
  // Coordinates
  ////////////////////////////////////////////////////////////////////////////////
  // GridImageSize
  auto grid_image_size_1=GridImageSize(1,1);
  auto grid_image_size_2=GridImageSize(3,4);
  GridImageSize grid_image_size_3=grid_image_size_1+grid_image_size_2;
  CHECK(grid_image_size_3.w() == 4);
  CHECK(grid_image_size_3.h() == 5);
  // GridCoordinateSize
  auto grid_coordinate_size_1=GridCoordinateSize(1.125,2.875);
  auto grid_coordinate_size_2=GridCoordinateSize(2.625,7.5);
  GridCoordinateSize grid_coordinate_size_3=grid_coordinate_size_1+grid_coordinate_size_2;
  CHECK(grid_coordinate_size_3.w() == 3.75);
  CHECK(grid_coordinate_size_3.h() == 10.375);
  // GridCoordinate
  auto grid_coordinate_1=GridCoordinate(3.25,9.125);
  auto grid_coordinate_2=GridCoordinate(4.625,2.875);
  GridCoordinate grid_coordinate_3=grid_coordinate_1+grid_coordinate_2;
  CHECK(grid_coordinate_3.x() == 7.875);
  CHECK(grid_coordinate_3.y() == 12.0);
  // GridIndex
  auto grid_index_1=GridIndex(2,3);
  auto grid_index_2=GridIndex(5,6);
  GridIndex grid_index_3=grid_index_1+grid_index_2;
  CHECK(grid_index_3.i() == 7);
  CHECK(grid_index_3.j() == 9);
  // SubGridImageSize
  auto subgrid_index_1=SubGridIndex(3,4);
  auto subgrid_index_2=SubGridIndex(5,6);
  SubGridIndex subgrid_index_3=subgrid_index_1+subgrid_index_2;
  CHECK(subgrid_index_3.i() == 8);
  CHECK(subgrid_index_3.j() == 10);
  // GridPixelSize
  auto grid_pixel_size_1=GridPixelSize(300,400);
  auto grid_pixel_size_2=GridPixelSize(900,1000);
  GridPixelSize grid_pixel_size_3=grid_pixel_size_1+grid_pixel_size_2;
  CHECK(grid_pixel_size_3.w() == 1200);
  CHECK(grid_pixel_size_3.h() == 1400);
  // ImagePixelCoordinate
  auto image_pixel_coordinate_1=ImagePixelCoordinate(345,890);
  auto image_pixel_coordinate_2=ImagePixelCoordinate(670,234);
  ImagePixelCoordinate image_pixel_coordinate_3=image_pixel_coordinate_1+image_pixel_coordinate_2;
  CHECK(image_pixel_coordinate_3.x() == 1015);
  CHECK(image_pixel_coordinate_3.y() == 1124);
  // BufferTileSize
  auto buffer_tile_size_1=BufferTileSize(1,2);
  auto buffer_tile_size_2=BufferTileSize(3,4);
  BufferTileSize buffer_tile_size_3=buffer_tile_size_1+buffer_tile_size_2;
  CHECK(buffer_tile_size_3.w() == 4);
  CHECK(buffer_tile_size_3.h() == 6);
  // BufferTileIndex
  auto buffer_tile_index_1=BufferTileIndex(3,9);
  auto buffer_tile_index_2=BufferTileIndex(4,2);
  BufferTileIndex buffer_tile_index_3=buffer_tile_index_1+buffer_tile_index_2;
  CHECK(buffer_tile_index_3.i() == 7);
  CHECK(buffer_tile_index_3.j() == 11);
  // BufferPixelSize
  auto buffer_pixel_size_1=BufferPixelSize(492,509);
  auto buffer_pixel_size_2=BufferPixelSize(623,456);
  BufferPixelSize buffer_pixel_size_3=buffer_pixel_size_1+buffer_pixel_size_2;
  CHECK(buffer_pixel_size_3.w() == 1115);
  CHECK(buffer_pixel_size_3.h() == 965);
  // BufferPixelCoordinate
  auto buffer_pixel_coordinate_1=BufferPixelCoordinate(230,823);
  auto buffer_pixel_coordinate_2=BufferPixelCoordinate(345,870);
  BufferPixelCoordinate buffer_pixel_coordinate_3=buffer_pixel_coordinate_1+buffer_pixel_coordinate_2;
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

TEST_CASE("Do images load correctly?") {

}

TEST_CASE("Does buffer copy work correctly?") {

}
