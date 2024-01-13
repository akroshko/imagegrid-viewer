/**
 * Implementation of the classes representing the texture grid of
 * (zoomed) textures.
 */
// local headers
#include "common.hpp"
#include "imagegrid/gridsetup.hpp"
#include "texturegrid.hpp"
// C compatible headers
#include "c_sdl2/sdl2.hpp"
// C++ headers
#include <atomic>
// C headers
#include <climits>
#include <cmath>

TextureGridSquareZoomLevel::TextureGridSquareZoomLevel (TextureGridSquare* parent_square,
                                                        const GridPixelSize& image_max_pixel_size,
                                                        INT64 zoom_out_shift) {
  GridPixelSize texture_display_size=static_cast<GridPixelSize>(image_max_pixel_size >> zoom_out_shift);
  // TODO: put in function along with similar functionality in texture_update.cpp
  auto texture_tile_size=TILE_PIXEL_BASE_SIZE;
  auto next_texture_tile_size=texture_tile_size >> 1L;
  while ((next_texture_tile_size > texture_display_size.w()) ||
         (next_texture_tile_size > texture_display_size.h())) {
    texture_tile_size >>= 1L;
    next_texture_tile_size >>= 1L;
  }
  auto tile_w=texture_display_size.w()/texture_tile_size;
  if (texture_display_size.w() % texture_tile_size != 0) {
    tile_w+=1;
  }
  auto tile_h=texture_display_size.h()/texture_tile_size;
  if (texture_display_size.h() % texture_tile_size != 0) {
    tile_h+=1;
  }
  this->_tile_size=BufferTileSize(tile_w,tile_h);
  this->_display_texture_wrapper.init(BufferTileSize(tile_w,tile_h));
  for (INT64 j=0; j < tile_h; j++) {
    for (INT64 i=0; i < tile_w; i++) {
      auto tile_index=BufferTileIndex(i,j);
      this->_display_texture_wrapper.set(tile_index,std::make_unique<SDLDisplayTextureWrapper>());
    }
  }
  this->_parent_square=parent_square;
}

TextureGridSquareZoomLevel::~TextureGridSquareZoomLevel () {
  this->unload_all_textures();
}

void TextureGridSquareZoomLevel::unload_all_textures () {
  for (INT64 j=0; j < this->_tile_size.h(); j++) {
    for (INT64 i=0; i < this->_tile_size.w(); i++) {
      auto tile_index=BufferTileIndex(i,j);
      if (this->_display_texture_wrapper[tile_index]->is_valid()) {
        this->_display_texture_wrapper[tile_index]->unload_surface();
      }
    }
  }
  this->is_loaded=false;
  this->is_displayable=false;
  this->last_load_index=INT_MAX;
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

bool TextureGridSquareZoomLevel::image_filler () const {
  return !this->is_loaded && this->is_displayable;
}

SDLDisplayTextureWrapper* TextureGridSquareZoomLevel::display_texture_wrapper(const BufferTileIndex& tile_index) {
  return this->_display_texture_wrapper[tile_index];
}

void TextureGridSquareZoomLevel::create_surfaces(INT64 tile_pixel_size) {
  for (INT64 j=0; j < this->_tile_size.h(); j++) {
    for (INT64 i=0; i < this->_tile_size.w(); i++) {
      auto tile_index=BufferTileIndex(i,j);
      if (!this->_display_texture_wrapper[tile_index]->is_valid()) {
        this->_display_texture_wrapper[tile_index]->create_surface(tile_pixel_size, tile_pixel_size);
      }
    }
  }
}

bool TextureGridSquareZoomLevel::all_surfaces_valid () {
  auto all_valid=true;
  for (INT64 j=0; j < this->_tile_size.h(); j++) {
    for (INT64 i=0; i < this->_tile_size.w(); i++) {
      auto tile_index=BufferTileIndex(i,j);
      if (!this->_display_texture_wrapper[tile_index]->is_valid()) {
        all_valid=false;
      };
    }
  }
  return all_valid;
}

bool TextureGridSquareZoomLevel::lock_surface (const BufferTileIndex& tile_index) {
  auto lock_surface_return=this->_display_texture_wrapper[tile_index]->lock_surface();
  return (!lock_surface_return);
}

void TextureGridSquareZoomLevel::unlock_surface (const BufferTileIndex& tile_index) {
  this->_display_texture_wrapper[tile_index]->unlock_surface();
}

bool TextureGridSquareZoomLevel::lock_all_surfaces () {
  auto all_lock_successful=true;
  for (INT64 j=0; j < this->_tile_size.h(); j++) {
    for (INT64 i=0; i < this->_tile_size.w(); i++) {
      auto tile_index=BufferTileIndex(i,j);
      auto lock_surface_return=this->_display_texture_wrapper[tile_index]->lock_surface();
      if (!lock_surface_return) {
        all_lock_successful=false;
      }
    }
  }
  return all_lock_successful;
}

void TextureGridSquareZoomLevel::unlock_all_surfaces () {
  for (INT64 j=0; j < this->_tile_size.h(); j++) {
    for (INT64 i=0; i < this->_tile_size.w(); i++) {
      auto tile_index=BufferTileIndex(i,j);
      this->_display_texture_wrapper[tile_index]->unlock_surface();
    }
  }
}

void TextureGridSquareZoomLevel::clear_all_surfaces () {
  for (INT64 j=0; j < this->_tile_size.h(); j++) {
    for (INT64 i=0; i < this->_tile_size.w(); i++) {
      auto tile_index=BufferTileIndex(i,j);
      this->_display_texture_wrapper[tile_index]->clear();
    }
  }
}

PIXEL_RGBA* TextureGridSquareZoomLevel::get_rgba_pixels(const BufferTileIndex& tile_index) {
  // TODO: definitely want an assert here
  return (PIXEL_RGBA*)this->_display_texture_wrapper[tile_index]->pixels();
}

BufferTileSize TextureGridSquareZoomLevel::tile_size() {
  return this->_tile_size;
}

BufferPixelSize TextureGridSquareZoomLevel::texture_square_pixel_size () const {
  return this->_texture_display_size;
}

TextureGridSquare::TextureGridSquare (TextureGrid* parent_grid,
                                      const GridPixelSize& image_max_pixel_size,
                                      INT64 zoom_out_shift_length) {
  this->_parent_grid=parent_grid;
  this->_zoom_out_shift_length=zoom_out_shift_length;
  this->texture_array.init(zoom_out_shift_length);
  for (auto i=0L; i < zoom_out_shift_length; i++) {
    this->texture_array.set(i,std::make_unique<TextureGridSquareZoomLevel>(this,
                                                                           image_max_pixel_size,
                                                                           i));
  }
}

TextureGrid* TextureGridSquare::parent_grid () const {
  return this->_parent_grid;
}

TextureGrid::TextureGrid (GridSetup* grid_setup,
                          const GridPixelSize& image_max_pixel_size,
                          INT64 zoom_out_shift_length) {
  this->_grid_setup=grid_setup;
  this->_grid_image_size=GridImageSize(grid_setup->grid_image_size());
  this->_zoom_out_shift_length=zoom_out_shift_length;
  this->_squares.init(grid_setup->grid_image_size());
  for (const auto& grid_index : ImageGridBasicIterator(this->_grid_setup)) {
    this->_squares.set(grid_index,std::make_unique<TextureGridSquare>(this,image_max_pixel_size,zoom_out_shift_length));
  }
}

GridSetup* TextureGrid::grid_setup() const {
  return this->_grid_setup;
};

TextureGridSquare* TextureGrid::squares(const GridIndex& grid_index) {
  return _squares[grid_index];
}

INT64 TextureGrid::textures_zoom_out_shift_length() const {
  return this->_zoom_out_shift_length;
}

GridImageSize TextureGrid::grid_image_size() const {
  return this->_grid_image_size;
}
