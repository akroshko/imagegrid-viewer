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
#include "c_compatible/sdl.hpp"
// C++ headers
#include <array>
#include <atomic>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

TextureGridSquareZoomLevel::TextureGridSquareZoomLevel () {
  this->display_texture_wrapper=std::make_unique<SDLDisplayTextureWrapper>();
}

TextureGridSquareZoomLevel::~TextureGridSquareZoomLevel () {
  // crashes for some reason, need to work this out, probably some other SDL deinitialization happening
  // this->unload_texture();
}

void TextureGridSquareZoomLevel::unload_texture () {
  if (this->display_texture_wrapper->is_valid()) {
    this->display_texture_wrapper->unload_surface();
    this->is_loaded=false;
    this->is_displayable=false;
    this->last_load_index=INT_MAX;
  }
}

void TextureGridSquareZoomLevel::set_image_loaded (INT_T load_index) {
  this->last_load_index=load_index;
  this->is_loaded=true;
  this->is_displayable=true;
}

void TextureGridSquareZoomLevel::set_image_filler () {
  this->is_loaded=false;
  this->is_displayable=true;
  this->last_load_index=INT_MAX;
}

TextureGridSquare::TextureGridSquare (INT_T zoom_index_length) {
  this->_zoom_index_length=zoom_index_length;
  this->texture_array=std::make_unique<TextureGridSquareZoomLevel*[]>(zoom_index_length);
  for (auto i=0L; i < zoom_index_length; i++) {
    this->texture_array[i]=new TextureGridSquareZoomLevel();
  }
}

TextureGridSquare::~TextureGridSquare () {
  for (auto i=0u; i < this->_zoom_index_length; i++) {
    DELETE_IF_NOT_NULLPTR(this->texture_array[i]);
  }
}

TextureGrid::TextureGrid (const GridSetup* const grid_setup,
                          INT_T zoom_index_length) {
  this->_grid_image_size=GridImageSize(grid_setup->grid_image_size());
  this->_zoom_index_length=zoom_index_length;
  this->squares=std::make_unique<TextureGridSquare**[]>(grid_setup->grid_image_size().wimage());
  for (INT_T i=0L; i < grid_setup->grid_image_size().wimage(); i++) {
    this->squares[i]=new TextureGridSquare*[grid_setup->grid_image_size().himage()];
    for (INT_T j=0L; j < grid_setup->grid_image_size().himage(); j++) {
      this->squares[i][j]=new TextureGridSquare(zoom_index_length);
    }
  }
}

TextureGrid::~TextureGrid() {
  for (auto i=0L; i < this->_grid_image_size.wimage(); i++) {
    for (INT_T j=0L; j < this->grid_image_size().himage(); j++) {
      DELETE_IF_NOT_NULLPTR(this->squares[i][j]);
    }
    DELETE_ARRAY_IF_NOT_NULLPTR(this->squares[i]);
  }
}

INT_T TextureGrid::textures_zoom_index_length() const {
  return this->_zoom_index_length;
}

GridImageSize TextureGrid::grid_image_size() const {
  return this->_grid_image_size;
}
