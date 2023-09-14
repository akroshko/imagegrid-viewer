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
#include "cdata/fileload.hpp"
#include "cinterface/sdl.hpp"
// C++ headers
#include <array>
#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>
// C headers
#include <cmath>

TextureGridSquareZoomLevel::TextureGridSquareZoomLevel () {
  this->_display_texture_wrapper=std::make_unique<SDLDisplayTextureWrapper>();
}

TextureGridSquareZoomLevel::~TextureGridSquareZoomLevel () {
  this->unload_texture();
}

void TextureGridSquareZoomLevel::unload_texture () {
  if (this->_display_texture_wrapper->is_valid()) {
    this->_display_texture_wrapper->unload_surface();
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

bool TextureGridSquareZoomLevel::get_image_filler () const {
  return !this->is_loaded && this->is_displayable;
}

SDLDisplayTextureWrapper* const TextureGridSquareZoomLevel::display_texture_wrapper() {
  return this->_display_texture_wrapper.get();
}
SDLDisplayTextureWrapper* const TextureGridSquareZoomLevel::filler_texture_wrapper() {
  return this->_filler_texture_wrapper;
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
  this->filler_squares=std::make_unique<SDLDisplayTextureWrapper*[]>(zoom_index_length);
}

void TextureGrid::init_filler_squares(const GridSetup* const grid_setup,
                                      INT_T zoom_index_length,
                                      GridPixelSize grid_pixel_size) {
  // fill squares
  // TODO: I may want to defer filling texture squares eventually
  // TODO: this should be built by the time things are ready
  //       may want to add an atomic
  for (INT_T zoom_index=0L; zoom_index < zoom_index_length; zoom_index++) {
    auto texture_zoom_reduction=((INT_T)pow(2,zoom_index));
    auto dest_wpixel=grid_pixel_size.wpixel()/texture_zoom_reduction;
    auto dest_hpixel=grid_pixel_size.hpixel()/texture_zoom_reduction;
    this->filler_squares[zoom_index] = new SDLDisplayTextureWrapper();
    this->filler_squares[zoom_index]->create_surface(dest_wpixel, dest_hpixel);
    auto lock_surface_return=this->filler_squares[zoom_index]->lock_surface();
    if (lock_surface_return == 0) {
      auto dest_array=this->filler_squares[zoom_index]->pixels();
      // copy over gray
      for (INT_T l=0L; l < dest_hpixel; l++) {
        for (INT_T k=0L; k < dest_wpixel; k++) {
          auto dest_index=(l*dest_wpixel+k)*3;
          ((unsigned char *)dest_array)[dest_index]=FILLER_LEVEL;
          ((unsigned char *)dest_array)[dest_index+1]=FILLER_LEVEL;
          ((unsigned char *)dest_array)[dest_index+2]=FILLER_LEVEL;
        }
      }
      this->filler_squares[zoom_index]->unlock_surface();
    }
  }
  for (INT_T zoom_index=0L; zoom_index < zoom_index_length; zoom_index++) {
    for (INT_T i=0L; i < grid_setup->grid_image_size().wimage(); i++) {
      for (INT_T j=0L; j < grid_setup->grid_image_size().himage(); j++) {
        this->squares[i][j]->texture_array[zoom_index]->_filler_texture_wrapper=this->filler_squares[zoom_index];
      }
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
