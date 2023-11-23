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
  GridPixelSize texture_display_size=image_max_pixel_size >> zoom_out_shift;
  // TODO: put in function along with simalur functionality in texture_update.cpp
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
  auto tile_num=tile_w*tile_h;
  this->_tile_size=BufferTileSize(tile_w,tile_h);
  this->_display_texture_wrapper=std::make_unique<std::unique_ptr<SDLDisplayTextureWrapper>[]>(tile_num);
  for (INT64 i=0; i < tile_num; i++) {
    this->_display_texture_wrapper[i]=std::make_unique<SDLDisplayTextureWrapper>();
  }
  this->_parent_square=parent_square;
}

TextureGridSquareZoomLevel::~TextureGridSquareZoomLevel () {
  this->unload_all_textures();
}

void TextureGridSquareZoomLevel::unload_all_textures () {
  auto tile_num=this->_tile_size.w()*this->_tile_size.h();
  for (INT64 i=0; i < tile_num; i++) {
    if (this->_display_texture_wrapper[i]->is_valid()) {
      this->_display_texture_wrapper[i]->unload_surface();
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

SDLDisplayTextureWrapper* TextureGridSquareZoomLevel::display_texture_wrapper(const BufferTileIndex& index) {
  auto tile_index=index.j()*this->_tile_size.w()+index.i();
  return this->_display_texture_wrapper[tile_index].get();
}

void TextureGridSquareZoomLevel::create_surfaces(INT64 tile_pixel_size) {
  auto tile_num=this->_tile_size.w()*this->_tile_size.h();
  for (INT64 i=0; i < tile_num; i++) {
    if (!this->_display_texture_wrapper[i]->is_valid()) {
      this->_display_texture_wrapper[i]->create_surface(tile_pixel_size, tile_pixel_size);
    }
  }
}

bool TextureGridSquareZoomLevel::all_surfaces_valid () {
  auto all_valid=true;
  auto tile_num=this->_tile_size.w()*this->_tile_size.h();
  for (INT64 i=0; i < tile_num; i++) {
    if (!this->_display_texture_wrapper[i]->is_valid()) {
      all_valid=false;
    };
  }
  return all_valid;
}

bool TextureGridSquareZoomLevel::lock_surface (const BufferTileIndex& index) {
  auto tile_index=index.j()*this->_tile_size.w()+index.i();
  auto lock_surface_return=this->_display_texture_wrapper[tile_index]->lock_surface();
  return (!lock_surface_return);
}

void TextureGridSquareZoomLevel::unlock_surface (const BufferTileIndex& index) {
  auto tile_index=index.j()*this->_tile_size.w()+index.i();
  this->_display_texture_wrapper[tile_index]->unlock_surface();
}

bool TextureGridSquareZoomLevel::lock_all_surfaces () {
  auto all_lock_successful=true;
  auto tile_num=this->_tile_size.w()*this->_tile_size.h();
  for (INT64 i=0; i < tile_num; i++) {
    auto lock_surface_return=this->_display_texture_wrapper[i]->lock_surface();
    if (!lock_surface_return) {
      all_lock_successful=false;
    }
  }
  return all_lock_successful;
}

void TextureGridSquareZoomLevel::unlock_all_surfaces () {
  auto tile_num=this->_tile_size.w()*this->_tile_size.h();
  for (INT64 i=0; i < tile_num; i++) {
    this->_display_texture_wrapper[i]->unlock_surface();
  }
}

void TextureGridSquareZoomLevel::clear_all_surfaces () {
  auto tile_num=this->_tile_size.w()*this->_tile_size.h();
  for (INT64 i=0; i < tile_num; i++) {
    this->_display_texture_wrapper[i]->clear();
  }
}

SDLDisplayTextureWrapper* TextureGridSquareZoomLevel::filler_texture_wrapper() {
  return this->_filler_texture_wrapper;
}

PIXEL_RGBA* TextureGridSquareZoomLevel::get_rgba_pixels(const BufferTileIndex& index) {
  // TODO: definitely want an assert here
  auto tile_index=index.j()*this->_tile_size.w()+index.i();
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
  this->texture_array=std::make_unique<std::unique_ptr<TextureGridSquareZoomLevel>[]>(zoom_out_shift_length);
  for (auto i=0L; i < zoom_out_shift_length; i++) {
    this->texture_array[i]=std::make_unique<TextureGridSquareZoomLevel>(this,
                                                                        image_max_pixel_size,
                                                                        i);
  }
}

TextureGrid* TextureGridSquare::parent_grid () const {
  return this->_parent_grid;
}

TextureGrid::TextureGrid (const GridSetup* const grid_setup,
                          const GridPixelSize& image_max_pixel_size,
                          INT64 zoom_out_shift_length) {
  this->_grid_image_size=GridImageSize(grid_setup->grid_image_size());
  this->_zoom_out_shift_length=zoom_out_shift_length;
  this->squares=std::make_unique<std::unique_ptr<std::unique_ptr<TextureGridSquare>[]>[]>(grid_setup->grid_image_size().w());
  for (INT64 i=0L; i < grid_setup->grid_image_size().w(); i++) {
    this->squares[i]=std::make_unique<std::unique_ptr<TextureGridSquare>[]>(grid_setup->grid_image_size().h());
    for (INT64 j=0L; j < grid_setup->grid_image_size().h(); j++) {
      this->squares[i][j]=std::make_unique<TextureGridSquare>(this,image_max_pixel_size,zoom_out_shift_length);
    }
  }
  this->filler_squares=std::make_unique<std::unique_ptr<SDLDisplayTextureWrapper>[]>(zoom_out_shift_length);
}

void TextureGrid::init_filler_squares(const GridSetup* const grid_setup,
                                      INT64 zoom_out_shift_length,
                                      const GridPixelSize& grid_pixel_size) {
  // fill squares
  // TODO: I may want to defer filling texture squares eventually
  // TODO: this should be built by the time things are ready
  //       may want to add an atomic
  for (INT64 zoom_out_shift=0L; zoom_out_shift < zoom_out_shift_length; zoom_out_shift++) {
    auto texture_zoom_reduction=(1L << zoom_out_shift);
    GridPixelSize dest_pixel_size=grid_pixel_size/texture_zoom_reduction;
    this->filler_squares[zoom_out_shift] = std::make_unique<SDLDisplayTextureWrapper>();
    this->filler_squares[zoom_out_shift]->create_surface(dest_pixel_size.w(), dest_pixel_size.h());
    auto lock_surface_return=this->filler_squares[zoom_out_shift]->lock_surface();
    if (lock_surface_return) {
      auto dest_array=this->filler_squares[zoom_out_shift]->pixels();
      auto wpixel_aligned=this->filler_squares[zoom_out_shift]->texture_size_aligned().w();
      auto hpixel_aligned=this->filler_squares[zoom_out_shift]->texture_size_aligned().h();
      // copy over gray
      for (INT64 l=0L; l < hpixel_aligned; l++) {
        for (INT64 k=0L; k < wpixel_aligned; k++) {
          auto dest_index=(l*wpixel_aligned+k);
          ((PIXEL_RGBA*)dest_array)[dest_index]=FILLER_LEVEL;
        }
      }
      this->filler_squares[zoom_out_shift]->unlock_surface();
    }
  }
  for (INT64 zoom_out_shift=0L; zoom_out_shift < zoom_out_shift_length; zoom_out_shift++) {
    for (INT64 i=0L; i < grid_setup->grid_image_size().w(); i++) {
      for (INT64 j=0L; j < grid_setup->grid_image_size().h(); j++) {
        this->squares[i][j]->texture_array[zoom_out_shift]->_filler_texture_wrapper=this->filler_squares[zoom_out_shift].get();
      }
    }
  }
}

INT64 TextureGrid::textures_zoom_out_shift_length() const {
  return this->_zoom_out_shift_length;
}

GridImageSize TextureGrid::grid_image_size() const {
  return this->_grid_image_size;
}
