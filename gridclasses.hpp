/**
 * Header file for the main classes representing the grid.  Includes
 * both loaded images and (zoomed) textures.
 */
#ifndef GRIDCLASSES_HPP
#define GRIDCLASSES_HPP

#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "defaults.hpp"
#include "coordinates.hpp"
#include "gridsetup.hpp"
// C++ headers
#include <vector>
#include <iostream>
#include <string>
#include <mutex>

// library headers
#include <SDL2/SDL.h>

/**
 * An individual square on the grid at a particular zoom level.
 *
 * TODO: Currently only stores one zoom level.  However code is in the
 * works that improves this.
 */
class ImageGridSquareZoomLevel {
public:
  ImageGridSquareZoomLevel()=default;
  ~ImageGridSquareZoomLevel();
  std::mutex load_mutex;
  // TODO: these don't use an object from coordinates.hpp since they
  // are raw memory
  /** */
  size_t rgb_wpixel;
  /** */
  size_t rgb_hpixel;
  // the actual RGB data
  unsigned char* rgb_data=nullptr;
};


/**
 * An individual square on the grid.
 */
class ImageGridSquare {
public:
  ImageGridSquare();
  ~ImageGridSquare();
  /**
   * Load a file into this square.
   *
   * @param The filename to load.
   */
  void read_file(std::string filename);
  void load_file(std::string filename);
  ImageGridSquareZoomLevel **image_array=nullptr;
};

/**
 * The grid of images.  In the future these will be lazily loaded from
 * disk/cache.
 */
class ImageGrid {
public:
  ImageGrid(GridSetup *grid_setup);
  ~ImageGrid();
  bool read_grid_info(GridSetup *grid_setup);
  bool load_grid(GridSetup *grid_setup);
  /** Store the coordinates */
  GridImageSize* grid_image_size=nullptr;
  /** Maximum size of images loaded into the grid. */
  GridPixelSize *image_max_size=nullptr;
  /** The individual squares in the image grid. */
  ImageGridSquare** squares=nullptr;
};

/**
 * An individual square at a particular zoom level in the texture
 * grid.
 */
class TextureGridSquareZoomLevel {
public:
  TextureGridSquareZoomLevel();
  ~TextureGridSquareZoomLevel();
  // lock when the display_area is being worked on
  std::mutex display_mutex;
  SDL_Surface* display_texture=nullptr;
};

/**
 * An individual square in the texture grid.
 */
class TextureGridSquare {
public:
  TextureGridSquare();
  ~TextureGridSquare();
  /**
   * An array of textures. The first element of the array is the
   * full-size texture the subsequent elements are zoomed textures
   * each reduced by a factor of 2.
   */
  TextureGridSquareZoomLevel** texture_array=nullptr;
  // TODO: does not work for now because elements of texture_array contain a mutex
  //       this can be improved
  // std::array<TextureGridSquareZoomLevel*, 10> texture_array;
  /** the size of the texture with no zoom */
  GridPixelSize* texture_pixel_size=nullptr;
};

/**
 * The grid as textures.  These are generally loaded lazily.
 */
class TextureGrid {
public:
  TextureGrid(GridSetup *grid_setup);
  ~TextureGrid();
  /**
   * Initialize the maximum size of each texture and the maximum zoom,
   * generally has to be done after all imagesare loaded.
   */
  void init_max_zoom_index(ImageGrid *grid);
  /**
   * Load a texture.
   *
   * @param source_square the square containing the RGB data
   *
   * @param dest_square the destination square to load the texture into
   *
   * @param zoom_level
   */
  bool load_texture(TextureGridSquare &dest_square,
                    ImageGridSquare &source_square,
                    INT_T zoom_level);
  /**
   * Update the textures based on the current coordinates and zoom
   * level.
   *
   * @param grid the image grid
   *
   * @param xgrid the x coordinate on the grid
   *
   * @param ygrid the y coordinate on the grid
   *
   * @param loadall load all textures at this zoom level, otherwise a 3x3 is loaded
   */
  void update_textures(ImageGrid *grid,
                       GridCoordinate *grid_coordinate,
                       INT_T zoom_level,
                       bool load_all);
  /** the indidivual squares */
  TextureGridSquare** squares=nullptr;
  /** this size of this grid in number of textures */
  GridImageSize *grid_image_size=nullptr;
  /** maximum size of the individual textures in pixels */
  GridPixelSize* max_pixel_size=nullptr;
  /** the maximum zoom (maximum number of reductions by a factor of 2) */
  INT_T textures_max_zoom_index;
};

#endif
