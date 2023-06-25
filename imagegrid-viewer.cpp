/**
 * The main program file for imagegrid-viewer.
 *
 * General overview of this program:
 *
 * The imagegrid-viewer program is meant to view many potentially
 * extremely large images that can be laid out in a grid.  It began
 * from wanting to have a software equivalent to "laying out maps on a
 * huge table or the floor", given the fact that the Government of
 * Canada topo maps can be found for free.  The government of Canada
 * topo maps are at
 * https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/
 * Digital maps and other large images are of course available from
 * other sources.
 *
 * I found it difficult to view these maps from the government in the
 * way I want (in a similar manner to laying out paper maps on a table
 * or floor in a grid) in the PDF viewers, image viewers, GIS
 * programs, and open source large image viewers I could find.
 * Especially compared to the ease of scrolling around in a program
 * like Google Earth, where the content is chosen by Google and not
 * guaranteed to remain static.  This program is an attempt to solve
 * these problems for local image data.
 *
 * An overview of the main data structures and program flow is given
 * in the docstrings for the ImageGridViewerContext below.  As as well
 * as an overview of future work.
 *
 */
// local headers
#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "defaults.hpp"
#include "gridsetup.hpp"
#include "sdl.hpp"
#include "gridclasses.hpp"
#include "texture_update.hpp"
#include "viewport.hpp"
#include "fileload.hpp"
// C++ headers
#include <atomic>
#include <iostream>
#include <thread>
#include <utility>

/**
 * The ImageGridViewerContext class stores the data structures
 * representing the state of the program.  This allows for a future
 * consideration where this program becomes a library.  This could
 * also useful for if a feature is added to render two seperate
 * locations at once.
 */
class ImageGridViewerContext {
public:
  ImageGridViewerContext()=default;
  ~ImageGridViewerContext();
  /**
   * Intialize the image grid using a subclass of the GridSetup class.
   *
   * @param grid_setup The GridSetup class (and subclasses) read the
   *                   user specification of the grid and grid data.
   *                   Currently this consists of reading a list of
   *                   files on the command line and/or a directory
   *                   with numbered files.  In the future, subclasses
   *                   could find this information based on data such
   *                   as map coorindates, configuration files, or the
   *                   input from GUI could be considered.
   */
  bool initialize_grid(GridSetup *grid_setup);
  /** A backend neutral delay that occurs at the end of the main loop */
  void delay();
  /**
   * Object to store the state of SDL.
   *
   * The SDLApp class stores any data related to SDL (simple direct
   * layer, see https://www.libsdl.org/), which is used as the
   * graphics backend.  This nicely encapsulates the C++ -> C library
   * interface away from the rest of the program.
   */
  SDLApp sdl_app;
  /**
   * Represents the loaded images as a grid.
   *
   * The ImageGrid class stores the raw RGB data of images loaded from
   * disk.  Currently all specified images are loaded into memory at
   * once.  I currently have a git branch where work is underway to
   * load/unload/cache image data in a seperate thread.  When this
   * work is done this program should be able to view substantially
   * larger maps than it does now.
   */
  ImageGrid* grid = nullptr;
  /** Class representing the view the user has of the image grid.
   *
   * The ViewPort class contains the state of the viewing window and
   * is responsible for drawing based on this state.  This state can
   * include the location where it is looking on the grid of images,
   * the zoom level, any move/zoom speed data, and any information on
   * user input.
   */
  ViewPort viewport = ViewPort(&viewport_current_state);
  /**
   * Stores textures at various zoom levels reflecting the needs of the viewport.
   *
   * The TextureGrid class stores zoomed textures ready to be drawn to
   * screen.  These are generated and freed as the user moves and
   * zooms around.  Each TextureGrid grid object, i.e.,
   * TextureGridSquareZoomLevel, contains a mutex so that textures are
   * not being updated and displayed at the same time.
   */
  TextureGrid* texture_grid = nullptr;
  /**
   * Class to the textures based on the current state of the viewport.
   *
   * The TextureUpdate class performs the update of the TextureGrid
   * class to reflect the current state of the viewport.  This runs in
   * its own thread and periodically locks TextureGridSquareZoomLevel
   * objects.  The least-zoomed textures are always available so
   * something can always be rendered.
   */
  TextureUpdate texture_update = TextureUpdate(&viewport_current_state);
  /**
   * Class used to transfer the current state of the viewport in a
   * threadsafe way.
   *
   * Data must be transferred between ViewPort and TextureUpdate
   * objects, which run in different threads.  The
   * ViewPortCurrentState class contains a mutex to help with this.
   * Once I have all the multithreaded components I want done (see
   * description of ImageGrid above) I will evaluate whether this is
   * the best technique.
   */
  ViewPortCurrentState viewport_current_state;
};

/**
 * Initialize the grid of an imagegrid-viewer program.
 *
 * @param grid_setup This contains the final configuration information
 *                   and files to setup the image grid.  This is
 *                   currently found by processing command line
 *                   arguments, hence other sources of interfaction
 *                   could be used to setup the command line
 *                   arguments.
 */
bool ImageGridViewerContext::initialize_grid(GridSetup *grid_setup) {
  this->grid=new ImageGrid(grid_setup);
  // this is where the images are loaded and can fail since it involves file I/O
  // TODO: work is in progress to load in a seperate thread
  auto read_images_successful=this->grid->read_grid_info(grid_setup);
  if (read_images_successful) {
    auto load_images_successful=this->grid->load_grid(grid_setup);
    if(load_images_successful) {
      // these depend on the images being loaded successfully but none
      // of these should not fail except in the case of OOM errors
      this->texture_grid=new TextureGrid(grid_setup);
      // find the maximum index to reference zoomed out textures
      // generally a heuristic based on image size and screen size
      this->texture_grid->init_max_zoom_index(this->grid);
      this->viewport.set_image_max_size(this->grid->image_max_size);
      // adjust initial position to a sensible default depending on how
      // many images are loaded
      this->viewport.adjust_initial_location(grid_setup);
    }
    return load_images_successful;
  }
  return read_images_successful;
};

ImageGridViewerContext::~ImageGridViewerContext() {
  if (this->grid != nullptr) {
    delete this->grid;
    this->grid=nullptr;
  }
  if (this->texture_grid != nullptr) {
    delete this->texture_grid;
    this->texture_grid=nullptr;
  }
}

/**
 * The delay function for the main loop will be backend-specific and
 * does not really belong anywhere else.
 *
 * Used at end of main loop to provide a delay.  Which is the standard
 * usage pattern of SDL.
 */
void ImageGridViewerContext::delay () {
  this->sdl_app.delay();
};

/**
 * Class to hold the thread that continually updates the texture data
 * based on where the viewport is looking.
 */
class UpdateTextureThread {
public:
  /**
   * Constructor to set up the class holding the thread.
   *
   * @param texture_update The TextureUpdate class this thread will
   *                       run.
   *
   * @param grid The ImageGrid class this thread will get images from.
   *
   * @param texture_grid The TextureGrid class that will hold the
   *                     scaled texture.
   */
  UpdateTextureThread(TextureUpdate *texture_update,
                      ImageGrid *grid,
                      TextureGrid *texture_grid) {
    this->_texture_update=texture_update;
    this->_grid=grid;
    this->_texture_grid=texture_grid;

  }
  UpdateTextureThread(const UpdateTextureThread&) = delete;
  UpdateTextureThread & operator=(const UpdateTextureThread&) = delete;
  /**
   * Start the thread itself.
   */
  std::thread start() {
    std::thread _worker_thread(&UpdateTextureThread::run, this);
    return _worker_thread;
  }
  /**
   * Terminate cleanly.  Joining needs to occur outside this scope for
   * now.
   */
  void terminate() {
    this->_keep_running=false;
  }
private:
  /**
   * Actually runs the function and holds the loop that updates the
   * textures.
   */
  void run () {
    MSG("Beginning thread in UpdateTextureThread.");
    while (this->_keep_running) {
      this->_texture_update->find_current_textures(this->_grid,
                                                   this->_texture_grid);
      // since we are lazily responding to user input, 10 milliseconds
      // should be fine for update interval
      std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }
    MSG("Ending execution in UpdateTextureThread.");
  }
  /** Flag to indicate whether the thread should keep running. */
  std::atomic<bool> _keep_running{true};
  std::thread _worker_thread;
  TextureUpdate *_texture_update;
  ImageGrid *_grid;
  TextureGrid *_texture_grid;
};

/**
 * The main function.
 *
 * Contains the initialization and main loop.
 *
 */
int main(int argc, char *argv[]) {
  // The main loop continues as long as this is true.
  auto continue_flag = true;
#ifdef DEBUG_MESSAGES
  int loop_count=0;
#endif
  // read the command line arguments to fine out what our grid looks
  // like and where the images come from
  auto grid_setup=new GridSetupFromCommandLine(argc,argv);
  if (!grid_setup->successful()) {
    if (grid_setup != nullptr) {
      delete grid_setup;
    }
    return 1;
  }
  // initialize application context
  auto imagegrid_viewer_context = new ImageGridViewerContext();
  // initialize SDL
  if (!imagegrid_viewer_context->sdl_app.successful()) {
    ERROR("Failed to SDL app initialize properly");
    if (grid_setup != nullptr) {
      delete grid_setup;
    }
    if (imagegrid_viewer_context != nullptr) {
      delete imagegrid_viewer_context;
    }
    return 1;
  }
  // initialialize the main data structures in the program, described
  // at the top of this file
  if (!imagegrid_viewer_context->initialize_grid(grid_setup)) {
    ERROR("Failed to load images!");
    if (grid_setup != nullptr) {
      delete grid_setup;
    }
    if (imagegrid_viewer_context != nullptr) {
      delete imagegrid_viewer_context;
    }
    return 1;
  }
  // TODO once the current development is done, see description at the
  // top of this file, I would like to get rid of the run input once

  // get input once before starting thread
  DEBUG("input() initial");
  continue_flag=imagegrid_viewer_context->viewport.do_input(
    &imagegrid_viewer_context->sdl_app);
  DEBUG("input() done");
  // update the current textures once before starting the thread
  DEBUG("find_current_textures() initial");
  imagegrid_viewer_context->texture_update.find_current_textures(
    imagegrid_viewer_context->grid,
    imagegrid_viewer_context->texture_grid);
  DEBUG("find_current_textures() initial done");
  // start the thread that updates textures
  auto update_texture_thread_class = std::make_unique<UpdateTextureThread>(
    &imagegrid_viewer_context->texture_update,
    imagegrid_viewer_context->grid,
    imagegrid_viewer_context->texture_grid);
  auto update_texture_thread = update_texture_thread_class->start();
  while (continue_flag) {
    DEBUG("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    DEBUG("++ Main loop begin");
    // read input, this also adjusts the coordinates of the viewport
    DEBUG("input()");
    continue_flag=imagegrid_viewer_context->viewport.do_input(&imagegrid_viewer_context->sdl_app);
    DEBUG("input() done");
    // find the textures that need to be blit to the viewport
    // if the textures haven't been loaded, a smaller zoomed version is used
    DEBUG("find_viewport_blit()");
    imagegrid_viewer_context->viewport.find_viewport_blit(
      imagegrid_viewer_context->texture_grid,
      &imagegrid_viewer_context->sdl_app);
    DEBUG("find_viewport_blit() done");
    // update the screen and delay before starting the loop again
    imagegrid_viewer_context->delay();
#ifdef DEBUG_MESSAGES
    DEBUG("Loop count: " << loop_count);
    loop_count++;
#endif
  }
  // allow update_texture_thread to cleanly terminate
  update_texture_thread_class->terminate();
  if (update_texture_thread.joinable()) {
    MSG("Joining update_texture_thread.");
    update_texture_thread.join();
    MSG("Finished joining update_texture_thread.");
  }
  if (grid_setup != nullptr) {
    delete grid_setup;
  }
  if (imagegrid_viewer_context != nullptr) {
    delete imagegrid_viewer_context;
  }
  return 0;
}
