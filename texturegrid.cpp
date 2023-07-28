/**
 * Implementation of the classes representing the texture grid of
 * (zoomed) textures.
 */
// local headers
#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "utility.hpp"
#include "gridsetup.hpp"
#include "imagegrid.hpp"
#include "texturegrid.hpp"
#include "iterators.hpp"
#include "viewport_current_state.hpp"
// C compatible headers
#include "c_compatible/fileload.hpp"
// C++ headers
#include <array>
#include <atomic>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

TextureGridSquareZoomLevel::~TextureGridSquareZoomLevel () {
  // crashes for some reason, need to work this out, probably some other SDL deinitialization happening
  // this->unload_texture();
}

void TextureGridSquareZoomLevel::unload_texture () {
  if (this->display_texture != nullptr) {
    SDL_FreeSurface(this->display_texture);
    this->display_texture=nullptr;
    this->last_load_index=INT_MAX;
  }
}

TextureGridSquare::TextureGridSquare () {
  this->texture_array=std::make_unique<TextureGridSquareZoomLevel*[]>(MAX_TEXTURE_ZOOM_INDEX);
  for (auto i=0ul; i < MAX_TEXTURE_ZOOM_INDEX; i++) {
    this->texture_array[i]=new TextureGridSquareZoomLevel();
  }
}

TextureGridSquare::~TextureGridSquare () {
  for (auto i=0ul; i < MAX_TEXTURE_ZOOM_INDEX; i++) {
    DELETE_IF_NOT_NULLPTR(this->texture_array[i]);
  }
}

TextureGrid::TextureGrid (const GridSetup* const grid_setup) {
  this->_grid_image_size=GridImageSize(grid_setup->grid_image_size());
  this->squares=std::make_unique<TextureGridSquare*[]>(grid_setup->grid_image_size().wimage());
  for (INT_T i=0l; i < grid_setup->grid_image_size().wimage(); i++) {
    this->squares[i]=new TextureGridSquare[grid_setup->grid_image_size().himage()];
  }
}

TextureGrid::~TextureGrid() {
  for (auto i=0l; i < this->_grid_image_size.wimage(); i++) {
    DELETE_ARRAY_IF_NOT_NULLPTR(this->squares[i]);
  }
}

void TextureGrid::init_max_zoom_index(const GridPixelSize &image_max_pixel_size) {
  INT_T zoom_length=0;
  FLOAT_T current_zoom=1.0;
  // swap out and reallocate
  this->_max_pixel_size=GridPixelSize(image_max_pixel_size);
  zoom_length += 1;
  current_zoom /= 2.0;
  // TODO: inefficient, do without a while loop
  while (!(((this->_grid_image_size.wimage()*this->max_pixel_size().wpixel()*current_zoom) < MAX_SCREEN_WIDTH) &&
           ((this->_grid_image_size.himage()*this->max_pixel_size().hpixel()*current_zoom) < MAX_SCREEN_HEIGHT))) {
    zoom_length += 1;
    current_zoom /= 2.0;
  }
  this->_textures_max_zoom_index=zoom_length-1;
}

INT_T TextureGrid::textures_max_zoom_index() const {
  return this->_textures_max_zoom_index;
}

GridPixelSize TextureGrid::max_pixel_size() const {
  return this->_max_pixel_size;
}

GridImageSize TextureGrid::grid_image_size() const {
  return this->_grid_image_size;
}
