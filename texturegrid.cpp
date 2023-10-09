/**
 * Implementation of the classes representing the texture grid of
 * (zoomed) textures.
 */
// local headers
#include "common.hpp"
#include "utility.hpp"
#include "imagegrid/gridsetup.hpp"
#include "texturegrid.hpp"
// C compatible headers
#include "c_sdl/sdl.hpp"
// C++ headers
#include <atomic>
// C headers
#include <cmath>

TextureGridSquareZoomLevel::TextureGridSquareZoomLevel (TextureGridSquare* parent_square) {
  this->_display_texture_wrapper=std::make_unique<SDLDisplayTextureWrapper>();
  this->_parent_square=parent_square;
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

SDLDisplayTextureWrapper* TextureGridSquareZoomLevel::display_texture_wrapper() {
  return this->_display_texture_wrapper.get();
}
SDLDisplayTextureWrapper* TextureGridSquareZoomLevel::filler_texture_wrapper() {
  return this->_filler_texture_wrapper;
}

TextureGridSquare::TextureGridSquare (TextureGrid* parent_grid,INT_T zoom_index_length) {
  this->_parent_grid=parent_grid;
  this->_zoom_index_length=zoom_index_length;
  this->texture_array=std::make_unique<std::unique_ptr<TextureGridSquareZoomLevel>[]>(zoom_index_length);
  for (auto i=0L; i < zoom_index_length; i++) {
    this->texture_array[i]=std::make_unique<TextureGridSquareZoomLevel>(this);
  }
}

TextureGrid* TextureGridSquare::parent_grid () const {
  return this->_parent_grid;
}

TextureGrid::TextureGrid (const GridSetup* const grid_setup,
                          INT_T zoom_index_length) {
  this->_grid_image_size=GridImageSize(grid_setup->grid_image_size());
  this->_zoom_index_length=zoom_index_length;
  this->squares=std::make_unique<std::unique_ptr<std::unique_ptr<TextureGridSquare>[]>[]>(grid_setup->grid_image_size().wimage());
  for (INT_T i=0L; i < grid_setup->grid_image_size().wimage(); i++) {
    this->squares[i]=std::make_unique<std::unique_ptr<TextureGridSquare>[]>(grid_setup->grid_image_size().himage());
    for (INT_T j=0L; j < grid_setup->grid_image_size().himage(); j++) {
      this->squares[i][j]=std::make_unique<TextureGridSquare>(this,zoom_index_length);
    }
  }
  this->filler_squares=std::make_unique<std::unique_ptr<SDLDisplayTextureWrapper>[]>(zoom_index_length);
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
    this->filler_squares[zoom_index] = std::make_unique<SDLDisplayTextureWrapper>();
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
        this->squares[i][j]->texture_array[zoom_index]->_filler_texture_wrapper=this->filler_squares[zoom_index].get();
      }
    }
  }
}

INT_T TextureGrid::textures_zoom_index_length() const {
  return this->_zoom_index_length;
}

GridImageSize TextureGrid::grid_image_size() const {
  return this->_grid_image_size;
}
