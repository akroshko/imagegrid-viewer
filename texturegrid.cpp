/**
 * Implementation of the classes representing the texture grid of
 * (zoomed) textures.
 */
// local headers
#include "common.hpp"
#include "imagegrid/gridsetup.hpp"
#include "texturegrid.hpp"
// C compatible headers
#include "c_sdl/sdl.hpp"
// C++ headers
#include <atomic>
// C headers
#include <climits>
#include <cmath>

TextureGridSquareZoomLevel::TextureGridSquareZoomLevel (TextureGridSquare* parent_square) {
  this->_parent_square=parent_square;
}

TextureGridSquareZoomLevel::~TextureGridSquareZoomLevel () {
  this->unload_all_textures();
}

void TextureGridSquareZoomLevel::unload_all_textures () {
  if (this->_tile_w > 0 && this->_tile_h > 0) {
    for (INT64 j=0; j < this->_tile_h; j++) {
      for (INT64 i=0; i < this->_tile_w; i++) {
        auto tile_index=j*this->_tile_w+i;
        if (this->_display_texture_wrapper[tile_index]->is_valid()) {
          this->_display_texture_wrapper[tile_index]->unload_surface();
        }
      }
    }
    for (INT64 i=0; i < this->_tile_w*this->_tile_h; i++) {
      this->_display_texture_wrapper[i].release();
    }
    this->_display_texture_wrapper.release();
    this->_tile_w=INT_MIN;
    this->_tile_h=INT_MIN;
    this->is_loaded=false;
    this->is_displayable=false;
    this->last_load_index=INT_MAX;
  }
}

void TextureGridSquareZoomLevel::set_image_loaded (INT64 load_index) {
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

SDLDisplayTextureWrapper* TextureGridSquareZoomLevel::display_texture_wrapper(INT64 tile_i, INT64 tile_j) {
  auto tile_index=tile_j*this->_tile_w+tile_i;
  return this->_display_texture_wrapper[tile_index].get();
}

void TextureGridSquareZoomLevel::create_surfaces(INT64 tile_w, INT64 tile_h,
                                                 INT64 tile_pixel_size) {
  this->_tile_w=tile_w;
  this->_tile_h=tile_h;
  // TODO: this should be an assertion
  if (this->_tile_w > 0 && this->_tile_h > 0) {
    // check higher up to make sure surfaces are freed before creating
    this->_display_texture_wrapper=std::make_unique<std::unique_ptr<SDLDisplayTextureWrapper>[]>(tile_w*tile_h);
    for (INT64 i=0; i < tile_w*tile_h; i++) {
      this->_display_texture_wrapper[i]=std::make_unique<SDLDisplayTextureWrapper>();
    }
    for (INT64 j=0; j < this->_tile_h; j++) {
      for (INT64 i=0; i < this->_tile_w; i++) {
        auto tile_index=j*this->_tile_w+i;
        this->_display_texture_wrapper[tile_index]->create_surface(tile_pixel_size, tile_pixel_size);
      }
    }
  }
}

bool TextureGridSquareZoomLevel::all_surfaces_valid () {
  auto all_valid=true;
  if (this->_tile_w > 0 && this->_tile_h > 0) {
    for (INT64 j=0; j < this->_tile_h; j++) {
      for (INT64 i=0; i < this->_tile_w; i++) {
        auto tile_index=j*this->_tile_w+i;
        if (!this->_display_texture_wrapper[tile_index]->is_valid()) {
          all_valid=false;
        };
      }
    }
  }
  return all_valid;
}

bool TextureGridSquareZoomLevel::lock_all_surfaces () {
  auto all_lock_successful=true;
  if (this->_tile_w > 0 && this->_tile_h > 0) {
    for (INT64 j=0; j < this->_tile_h; j++) {
      for (INT64 i=0; i < this->_tile_w; i++) {
        auto tile_index=j*this->_tile_w+i;
        auto lock_surface_return=this->_display_texture_wrapper[tile_index]->lock_surface();
        if (lock_surface_return != 0) {
          all_lock_successful=false;
        }
      }
    }
  }
  return all_lock_successful;
}

void TextureGridSquareZoomLevel::unlock_all_surfaces () {
  if (this->_tile_w > 0 && this->_tile_h > 0) {
    for (INT64 j=0; j < this->_tile_h; j++) {
      for (INT64 i=0; i < this->_tile_w; i++) {
        auto tile_index=j*this->_tile_w+i;
        this->_display_texture_wrapper[tile_index]->unlock_surface();
      }
    }
  }
}

SDLDisplayTextureWrapper* TextureGridSquareZoomLevel::filler_texture_wrapper() {
  return this->_filler_texture_wrapper;
}

void* TextureGridSquareZoomLevel::get_rgba_pixels(INT64 tile_i, INT64 tile_j) {
  // TODO: definitely want an assert here
  auto tile_index=tile_j*this->_tile_w+tile_i;
  return this->_display_texture_wrapper[tile_index]->pixels();
}

INT64 TextureGridSquareZoomLevel::tile_w() {
  return this->_tile_w;
}

INT64 TextureGridSquareZoomLevel::tile_h() {
  return this->_tile_h;
}

TextureGridSquare::TextureGridSquare (TextureGrid* parent_grid,INT64 zoom_index_length) {
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
                          INT64 zoom_index_length) {
  this->_grid_image_size=GridImageSize(grid_setup->grid_image_size());
  this->_zoom_index_length=zoom_index_length;
  this->squares=std::make_unique<std::unique_ptr<std::unique_ptr<TextureGridSquare>[]>[]>(grid_setup->grid_image_size().wimage());
  for (INT64 i=0L; i < grid_setup->grid_image_size().wimage(); i++) {
    this->squares[i]=std::make_unique<std::unique_ptr<TextureGridSquare>[]>(grid_setup->grid_image_size().himage());
    for (INT64 j=0L; j < grid_setup->grid_image_size().himage(); j++) {
      this->squares[i][j]=std::make_unique<TextureGridSquare>(this,zoom_index_length);
    }
  }
  this->filler_squares=std::make_unique<std::unique_ptr<SDLDisplayTextureWrapper>[]>(zoom_index_length);
}

void TextureGrid::init_filler_squares(const GridSetup* const grid_setup,
                                      INT64 zoom_index_length,
                                      GridPixelSize grid_pixel_size) {
  // fill squares
  // TODO: I may want to defer filling texture squares eventually
  // TODO: this should be built by the time things are ready
  //       may want to add an atomic
  for (INT64 zoom_index=0L; zoom_index < zoom_index_length; zoom_index++) {
    auto texture_zoom_reduction=((INT64)pow(2,zoom_index));
    auto dest_wpixel=grid_pixel_size.wpixel()/texture_zoom_reduction;
    auto dest_hpixel=grid_pixel_size.hpixel()/texture_zoom_reduction;
    this->filler_squares[zoom_index] = std::make_unique<SDLDisplayTextureWrapper>();
    this->filler_squares[zoom_index]->create_surface(dest_wpixel, dest_hpixel);
    auto lock_surface_return=this->filler_squares[zoom_index]->lock_surface();
    if (lock_surface_return == 0) {
      auto dest_array=this->filler_squares[zoom_index]->pixels();
      // copy over gray
      for (INT64 l=0L; l < dest_hpixel; l++) {
        for (INT64 k=0L; k < dest_wpixel; k++) {
          auto dest_index=(l*dest_wpixel+k);
          ((PIXEL_RGBA*)dest_array)[dest_index]=FILLER_LEVEL;
        }
      }
      this->filler_squares[zoom_index]->unlock_surface();
    }
  }
  for (INT64 zoom_index=0L; zoom_index < zoom_index_length; zoom_index++) {
    for (INT64 i=0L; i < grid_setup->grid_image_size().wimage(); i++) {
      for (INT64 j=0L; j < grid_setup->grid_image_size().himage(); j++) {
        this->squares[i][j]->texture_array[zoom_index]->_filler_texture_wrapper=this->filler_squares[zoom_index].get();
      }
    }
  }
}

INT64 TextureGrid::textures_zoom_index_length() const {
  return this->_zoom_index_length;
}

GridImageSize TextureGrid::grid_image_size() const {
  return this->_grid_image_size;
}
