/**
 * The main program file for imagegrid-viewer.
 *
 * General overview of this program:
 *
 * The imagegrid-viewer program is meant to view many potentially
 * extremely large images that can be laid out in a grid.  It began
 * from wanting to have a software equivalent to "laying out maps on a
 * huge table or the floor", given the fact that the Government of
 * Canada topographic maps can be found for free.  The government of
 * Canada topo maps are at
 * https://ftp.maps.canada.ca/pub/nrcan_rncan/raster/canmatrix2/
 * Digital maps and other large images are of course available from
 * other sources.
 *
 * I found it difficult to view these maps in the way I want (in a
 * similar manner to laying out paper maps on a table or floor in a
 * grid) in the PDF viewers, image viewers, GIS programs, and open
 * source large image viewers I could find.  Especially compared to
 * the ease of scrolling around in a program like Google Earth, where
 * the content is chosen by Google and not guaranteed to remain
 * static.  This program is an attempt to solve these problems for
 * local image data.
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
#include "utility.hpp"
#include "gridsetup.hpp"
#include "gridclasses.hpp"
#include "texture_update.hpp"
#include "viewport.hpp"
// C compatible headers
#include "c_compatible/sdl.hpp"
#include "c_compatible/fileload.hpp"
// C++ headers
#include <atomic>
#include <iostream>
#include <thread>
#include <utility>

/**
 * The ImageGridViewerContext class stores the data structures
 * representing the state of the program.  This allows for a future
 * consideration where this program becomes a library or a component
 * of a larger program.  This could also useful for if a feature is
 * added to render two seperate locations at once.
 */
class ImageGridViewerContext {
public:
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
  ImageGridViewerContext()=delete;
  ImageGridViewerContext(GridSetup *grid_setup);
  ImageGridViewerContext(const ImageGridViewerContext&)=delete;
  ImageGridViewerContext(const ImageGridViewerContext&&)=delete;
  ImageGridViewerContext& operator=(const ImageGridViewerContext&)=delete;
  ImageGridViewerContext& operator=(const ImageGridViewerContext&&)=delete;
  /** A backend neutral delay that occurs at the end of the main loop.
   *
   * This allows encapsulating the C interface to SDL away from the
   * main C++ program.
   */
  void delay();
  /**
   * Object to store the state of SDL.
   *
   * The SDLApp class stores any data related to SDL (simple direct
   * layer, see https://www.libsdl.org/), which is used as the
   * graphics backend.  This nicely encapsulates the C++ -> C library
   * interface away from the rest of the program.
   */
  std::unique_ptr<SDLApp> sdl_app;
  /**
   * Represents the loaded images as a grid.
   *
   * The ImageGrid class stores the raw RGB data of images loaded from
   * disk.  These images are sparsely loaded so they are not
   * guaranteed to exist.
   */
  std::unique_ptr<ImageGrid> grid;
  /** Class representing the view the user has of the image grid.
   *
   * The ViewPort class contains the state of the viewing window and
   * is responsible for drawing based on this state.  This state can
   * include the location where it is looking on the grid of images,
   * the zoom level, any move/zoom speed data, and any information on
   * user input.
   */
  std::unique_ptr<ViewPort> viewport;
  /**
   * Stores textures at various zoom levels reflecting the needs of the viewport.
   *
   * The TextureGrid class stores zoomed textures ready to be drawn to
   * screen.  These are generated and freed as the user moves and
   * zooms around.  Each TextureGrid grid object, i.e.,
   * TextureGridSquareZoomLevel, contains a mutex so that textures are
   * not being updated and displayed at the same time.
   */
  std::unique_ptr<TextureGrid> texture_grid;
  /**
   * Class to the textures based on the current state of the viewport.
   *
   * The TextureUpdate class performs the update of the TextureGrid
   * class to reflect the current state of the viewport.  This runs in
   * its own thread and periodically locks TextureGridSquareZoomLevel
   * objects.  The least-zoomed textures are always available so
   * something can always be rendered.
   */
  std::unique_ptr<TextureUpdate> texture_update;
  /**
   * Class used to transfer the current state of the viewport in a
   * threadsafe way.
   *
   * Data must be transferred from ViewPort to the TextureUpdate, and
   * ImageGrid objects, which run in different threads.  The
   * ViewPortCurrentState class contains a mutex to help with this.
   * Once I have all the multithreaded components I want done (see
   * description of ImageGrid above) I will evaluate whether this is
   * the best technique.
   */
  std::shared_ptr<ViewPortCurrentState> viewport_current_state_texturegrid_update;
  std::shared_ptr<ViewPortCurrentState> viewport_current_state_imagegrid_update;
  bool successful;
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
ImageGridViewerContext::ImageGridViewerContext(GridSetup *grid_setup) {
  this->sdl_app=std::make_unique<SDLApp>();
  this->viewport_current_state_texturegrid_update=std::make_shared<ViewPortCurrentState>();
  this->viewport_current_state_imagegrid_update=std::make_shared<ViewPortCurrentState>();
  this->viewport=std::make_unique<ViewPort>(this->viewport_current_state_texturegrid_update,
                                            this->viewport_current_state_imagegrid_update);
  this->texture_update=std::make_unique<TextureUpdate>(this->viewport_current_state_texturegrid_update);
  this->grid=std::make_unique<ImageGrid>(grid_setup,this->viewport_current_state_imagegrid_update);
  // this is where the the info on the grid is loaded
  // the actual image data is loaded seperately in it's own thread
  auto read_images_successful=this->grid->read_grid_info(grid_setup);
  if (read_images_successful) {
    this->texture_grid=std::make_unique<TextureGrid>(grid_setup);
    // find the maximum index to reference zoomed out textures
    // generally a heuristic based on image size and screen size
    this->texture_grid->init_max_zoom_index(this->grid->get_image_max_pixel_size());
    this->viewport->set_image_max_size(this->grid->get_image_max_pixel_size());
    // adjust initial position to a sensible default depending on how
    // many images are loaded
    this->viewport->adjust_initial_location(grid_setup);
  }
  this->successful=read_images_successful;
};

/**
 * The delay function for the main loop will be backend-specific and
 * does not really belong anywhere else.
 *
 * Used at end of main loop to provide a delay.  Which is the standard
 * usage pattern of SDL.
 */
void ImageGridViewerContext::delay () {
  this->sdl_app->delay();
};


/**
 * Class to hold the thread that continually updates the loaded data
 * for the ImageGrid.
 *
 */
class UpdateImageGridThread {
public:
  UpdateImageGridThread()=delete;
  /**
   * Constructor to set up the class holding the thread.
   *
   * @param grid_setup The object holding the data on the images in
   *                   the grid, including the filenames and grid
   *                   size.
   *
   * @param grid       The object holding the loaded image data.
   */
  UpdateImageGridThread(GridSetup *grid_setup, ImageGrid *grid) {
    this->_grid=grid;
    this->_grid_setup=grid_setup;
  }
  UpdateImageGridThread(const UpdateImageGridThread&)=delete;
  UpdateImageGridThread(const UpdateImageGridThread&&)=delete;
  UpdateImageGridThread& operator=(const UpdateImageGridThread&)=delete;
  UpdateImageGridThread& operator=(const UpdateImageGridThread&&)=delete;
  /**
   * Start the thread itself.
   */
  std::thread start() {
    std::thread _worker_thread(&UpdateImageGridThread::run, this);
    return _worker_thread;
  }
  /**
   * Terminate the thread.
   */
  void terminate() {
    this->_keep_running=false;
  }
  /**
   * Unload all files.  Generally used for testing.
   */
  void unload_files() {
  }
private:
  /**
   * Actually runs the function and holds the loop that updates the
   * textures.
   */
  void run () {
    // TODO fix this up so I can do a load_all
    // this->_all_loaded=true;
    while (this->_keep_running) {
      this->_grid->load_grid(this->_grid_setup,this->_keep_running);
      sleep_thread();
    }
    MSG("Ending execution in UpdateImageGridThread.");
  }
  ImageGrid *_grid;
  GridSetup *_grid_setup;
  std::atomic<bool> _keep_running{true};
  std::atomic<bool> _all_loaded{false};
  std::thread _worker_thread;
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
  UpdateTextureThread(const UpdateTextureThread&)=delete;
  UpdateTextureThread(const UpdateTextureThread&&)=delete;
  UpdateTextureThread& operator=(const UpdateTextureThread&)=delete;
  UpdateTextureThread& operator=(const UpdateTextureThread&&)=delete;
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
                                                   this->_texture_grid,
                                                   this->_keep_running);
      sleep_thread();
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
  auto continue_flag=true;
#ifdef DEBUG_MESSAGES
  int loop_count=0;
#endif
  // read the command line arguments to fine out what our grid looks
  // like and where the images come from
  auto grid_setup=std::make_unique<GridSetupFromCommandLine>(argc,argv);
  if (!grid_setup->successful()) {
    return 1;
  }
  auto imagegrid_viewer_context=std::make_unique<ImageGridViewerContext>(grid_setup.get());
  // initialize SDL
  if (!imagegrid_viewer_context->sdl_app->successful()) {
    ERROR("Failed to SDL app initialize properly");
    return 1;
  }
  // initialialize the main data structures in the program, described
  // at the top of this file
  if (!imagegrid_viewer_context->successful) {
    ERROR("Failed to find images!");
    return 1;
  }
  // start the thead that loads the imagegrid
  DEBUG("UpdateImageGridThread start()");
  auto update_imagegrid_thread_class=std::make_unique<UpdateImageGridThread>(
    grid_setup.get(),
    imagegrid_viewer_context->grid.get());
  auto update_imagegrid_thread=update_imagegrid_thread_class->start();
  // start the thread that updates textures
  auto update_texture_thread_class=std::make_unique<UpdateTextureThread>(
    imagegrid_viewer_context->texture_update.get(),
    imagegrid_viewer_context->grid.get(),
    imagegrid_viewer_context->texture_grid.get());
  auto update_texture_thread=update_texture_thread_class->start();
  while (continue_flag) {
    DEBUG("================================================================================");
    DEBUG("== Main loop begin");
    // read input, this also adjusts the coordinates of the viewport
    DEBUG("input()");
    continue_flag=imagegrid_viewer_context->viewport->do_input(imagegrid_viewer_context->sdl_app.get());
    DEBUG("input() end");
    // find the textures that need to be blit to the viewport
    // if the textures haven't been loaded, a smaller unzoomed version is used
    imagegrid_viewer_context->viewport->find_viewport_blit(
      imagegrid_viewer_context->texture_grid.get(),
      imagegrid_viewer_context->sdl_app.get());
    // update the screen and delay before starting the loop again
    imagegrid_viewer_context->delay();
#ifdef DEBUG_MESSAGES
    DEBUG("Loop count: " << loop_count);
    loop_count++;
#endif
  }
  // send termination signal to both threads
  update_imagegrid_thread_class->terminate();
  update_texture_thread_class->terminate();
  // wait for update_imagegrid_thread to cleanly terminate
  if (update_imagegrid_thread.joinable()) {
    MSG("Joining update_imagegrid_thread.");
    update_imagegrid_thread.join();
    MSG("Finished joining update_imagegrid_thread.");
  }
  // wait for update_texture_thread to cleanly terminate
  if (update_texture_thread.joinable()) {
    MSG("Joining update_texture_thread.");
    update_texture_thread.join();
    MSG("Finished joining update_texture_thread.");
  }
  return 0;
}
