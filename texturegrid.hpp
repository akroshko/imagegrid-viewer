/**
 * Header file for the main classes representing the grid of textures.
 */
#ifndef TEXTUREGRID_HPP
#define TEXTUREGRID_HPP

#include "common.hpp"
#include "coordinates.hpp"
#include "imagegrid/gridsetup.hpp"
#include "imagegrid/imagegrid.hpp"
#include "c_sdl2/sdl2.hpp"
// C++ headers
#include <atomic>
#include <memory>
#include <mutex>
// C headers
#include <climits>

class TextureGrid;
class TextureGridSquare;
class TextureGridSquareZoomLevel;

/**
 * An individual square at a particular zoom level in the texture
 * grid.
 */
class TextureGridSquareZoomLevel {
public:
  TextureGridSquareZoomLevel()=delete;
  TextureGridSquareZoomLevel(TextureGridSquare* parent_square,
                             const GridPixelSize& image_max_pixel_size,
                             INT64 zoom_out_shift);
  ~TextureGridSquareZoomLevel();
  TextureGridSquareZoomLevel(const TextureGridSquareZoomLevel&)=delete;
  TextureGridSquareZoomLevel(const TextureGridSquareZoomLevel&&)=delete;
  TextureGridSquareZoomLevel& operator=(const TextureGridSquareZoomLevel&)=delete;
  TextureGridSquareZoomLevel& operator=(const TextureGridSquareZoomLevel&&)=delete;
  /**
   * Unload the texture for this square.  Also resets any state.
   */
  void unload_all_textures();
  /**
   * Set this texture as a loaded image/data.
   *
   * @param load_index The last loaded index representing the zoom
   *                   level of what was loaded.
   */
  void set_image_loaded (INT64 load_index);
  /**
   * Set this texture as a filler.
   */
  void set_image_filler ();
  /**
   * Get whether this texture is a filler.
   *
   * @return Whether this texture is a filler.
   */
  bool get_image_filler () const;
  // lock when the display_area is being worked on
  std::mutex display_mutex;
  // is a real image/data loaded here
  std::atomic<bool> is_loaded{false};
  // is something loaded that makes this displayable, can be a filler texture
  std::atomic<bool> is_displayable{false};
  // TODO: this one needs help being private and investigation whether
  // there's a better way
  INT64 last_load_index=INT_MAX;
  // TODO: this will have to be made private
  ImageGridSquareZoomLevel* _source_square;
  /**
   * Create surfaces to render to.
   *
   * @param tile_pixel_size The pixel size of each tile (they are square).
   */
  void create_surfaces(INT64 tile_pixel_size);
  /** @return If all surfaces are valid. */
  bool all_surfaces_valid ();
  /**
   * Lock one surface.
   *
   * @param index The index of the surface to lock.
   * @return If the surface was locked successfully.
   */
  bool lock_surface (const BufferTileIndex& index);
  /** @return If locking all surfaces was successful. */
  bool lock_all_surfaces ();
  /**
   * Unlock one surface.

   * @param index The index of the surface to unlock.
   */
  void unlock_surface (const BufferTileIndex& index);
  /** Unlock all surfaces. */
  void unlock_all_surfaces ();
  /** Clear all surfaces. */
  void clear_all_surfaces ();
  /** Get the RGBA pixels for a particular tile. */
  PIXEL_RGBA* get_rgba_pixels(const BufferTileIndex& index);
  /**
   * Get the display texture for a tile.
   *
   * @param index The index of the rile to get the wrapper for.
   */
  SDLDisplayTextureWrapper* display_texture_wrapper(const BufferTileIndex& index);
  /**
   * Get the texture used for the filler texture.
   */
  SDLDisplayTextureWrapper* filler_texture_wrapper();
  /** @return The size in of the tiles. */
  BufferTileSize tile_size();
  /** @return The size in pixels that this square actually displays. */
  BufferPixelSize texture_square_pixel_size() const;
  // the size of the texture that actually gets displayed
  // this is so the padding and tile sizes do not affect what can be
  // displayed
  // TODO: decide whether TextureUpdate is a friend or do some other
  // solution
  BufferPixelSize _texture_display_size;
  // INT64 _texture_display_wpixel;
  // INT64 _texture_display_hpixel;
private:
  friend class TextureGrid;
  friend class TextureGridSquare;
  TextureGridSquare* _parent_square;
  BufferTileSize _tile_size;
  // the actual display texture
  std::unique_ptr<std::unique_ptr<SDLDisplayTextureWrapper>[]> _display_texture_wrapper;
  // the filler texture
  SDLDisplayTextureWrapper* _filler_texture_wrapper;
};

/**
 * An individual square in the texture grid.
 */
class TextureGridSquare {
public:
  TextureGridSquare()=delete;
  TextureGridSquare(TextureGrid* parent_grid,
                    const GridPixelSize& image_max_pixel_size,
                    INT64 zoom_index_length);
  ~TextureGridSquare()=default;
  TextureGridSquare(const TextureGridSquare&)=delete;
  TextureGridSquare(const TextureGridSquare&&)=delete;
  TextureGridSquare& operator=(const TextureGridSquare&)=delete;
  TextureGridSquare& operator=(const TextureGridSquare&&)=delete;
  /**
   * An array of textures. The first element of the array is the
   * full-size texture the subsequent elements are zoomed textures
   * each reduced by a factor of 2.
   */
  std::unique_ptr<std::unique_ptr<TextureGridSquareZoomLevel>[]> texture_array;
  /** @return The parent texture grid.*/
  TextureGrid* parent_grid() const;
private:
  friend class TextureGrid;
  friend class TextureGridSquareZoomLevel;
  TextureGrid* _parent_grid;
  INT64 _zoom_index_length;
};

/**
 * The grid as textures.  These are generally loaded lazily.
 */
class TextureGrid {
public:
  TextureGrid()=delete;
  /**
   *
   * @param grid_setup The object holding the data on the images in
   *                   the grid, including the filenames and grid
   *                   size.
   * @param image_max_pixel_size
   * @param zoom_index_length The length of the array holding
   *                          progressively zoomed out images.
   */
  TextureGrid(const GridSetup* grid_setup,
              const GridPixelSize& image_max_pixel_size,
              INT64 zoom_index_length);
  ~TextureGrid()=default;
  TextureGrid(const TextureGrid&)=delete;
  TextureGrid(const TextureGrid&&)=delete;
  TextureGrid& operator=(const TextureGrid&)=delete;
  TextureGrid& operator=(const TextureGrid&&)=delete;
  /** the individual squares */
  std::unique_ptr<std::unique_ptr<std::unique_ptr<TextureGridSquare>[]>[]> squares;
  /** fillers for the individual squares. */
  std::unique_ptr<std::unique_ptr<SDLDisplayTextureWrapper>[]> filler_squares;
  /** @return The size of the grid in images. */
  GridImageSize grid_image_size() const;
  /** @return The length of texture zoom array. */
  INT64 textures_zoom_index_length() const;
  /**
   * @param grid_setup The object holding the data on the images in
   *                   the grid, including the filenames and grid
   *                   size.
   * @param zoom_index_length The length of the array holding
   *                          progressively zoomed out images.
   * @param grid_pixel_size The maximum size of images in the grid.
   */
  void init_filler_squares(const GridSetup* const grid_setup,
                           INT64 zoom_index_length,
                           const GridPixelSize& grid_pixel_size);
private:
  /** this size of this grid in number of textures */
  GridImageSize _grid_image_size;
  /** the maximum zoom (maximum number of reductions by a factor of 2) */
  INT64 _zoom_index_length;
};

#endif
