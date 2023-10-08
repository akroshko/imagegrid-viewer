#include "common.hpp"
#include "imagegrid/gridsetup.hpp"
#include "coordinates.hpp"
#include "viewport.hpp"
#include "viewport_current_state.hpp"
// C compatible headers
#include "c_sdl/sdl.hpp"
// C++ headers
#include <vector>
// C headers
#include <cmath>

BlitItem::BlitItem(TextureGridSquareZoomLevel* const square, INT_T count,
                   const ViewportPixelCoordinate &l_viewport_pixel_coordinate,
                   const ViewportPixelSize &grid_image_size_zoomed) {
  this->_blit_index=count;
  this->blit_square=square;
  this->viewport_pixel_coordinate=ViewportPixelCoordinate(l_viewport_pixel_coordinate);
  this->image_pixel_size_viewport=ViewportPixelSize(grid_image_size_zoomed);
}

void BlitItem::blit_this(SDLDrawableSurface* screen_surface) {
  if (this->blit_square->get_image_filler()) {
    blit_square->filler_texture_wrapper()->blit_texture(screen_surface,
                                                        this->viewport_pixel_coordinate,
                                                        this->image_pixel_size_viewport);
  } else {
    blit_square->display_texture_wrapper()->blit_texture(screen_surface,
                                                         this->viewport_pixel_coordinate,
                                                         this->image_pixel_size_viewport);
  }
}

ViewPort::ViewPort(std::shared_ptr<ViewPortTransferState> viewport_current_state_texturegrid_update,
                   std::shared_ptr<ViewPortTransferState> viewport_current_state_imagegrid_update) {
  this->_viewport_current_state_texturegrid_update=viewport_current_state_texturegrid_update;
  this->_viewport_current_state_imagegrid_update=viewport_current_state_imagegrid_update;
  this->update_viewport_info(INITIAL_X,INITIAL_Y);
}

void ViewPort::set_image_max_size(const GridPixelSize& image_max_size) {
  this->_image_max_size=GridPixelSize(image_max_size);
}

void ViewPort::find_viewport_blit(TextureGrid* const texture_grid, SDLApp* const sdl_app) {
  INT_T blit_count=0;
  // locking the textures
  // std::vector<std::unique_lock<std::mutex>> mutex_vector{};
  std::vector<TextureGridSquareZoomLevel*> texture_square_vector{};
  // Stores the next items to be blit to the viewport.
  std::vector<std::unique_ptr<BlitItem>> blititems;
  // lifted out of the old find_viewport_extents_grid function since it's only called from here
  // will be put into function eventually, but I wanted to use new data structures
  // I don't use objects for some things because I want to use FLOAT_T for intermediate calculations
  auto half_width=this->_viewport_pixel_size.wpixel() / 2.0;
  auto half_height=this->_viewport_pixel_size.hpixel() / 2.0;
  auto viewport_left_distance_grid=(half_width/this->_image_max_size.wpixel()/this->_zoom);
  auto viewport_top_distance_grid=(half_height/this->_image_max_size.hpixel()/this->_zoom);
  auto viewport_left_grid=this->_viewport_grid.xgrid()-viewport_left_distance_grid;;
  auto viewport_top_grid=this->_viewport_grid.ygrid()-viewport_top_distance_grid;
  auto max_zoom_index=texture_grid->textures_zoom_index_length()-1;
  // TODO refactor this out
  if (max_zoom_index < 0) { max_zoom_index=0; }
  auto zoom_index=ViewPortTransferState::find_zoom_index_bounded(this->_zoom,0,max_zoom_index);
  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////
  // now loop over grid squares
  for (INT_T i=0L; i < texture_grid->grid_image_size().wimage(); i++) {
    for (INT_T j=0L; j < texture_grid->grid_image_size().himage(); j++) {
      auto gi=j*texture_grid->grid_image_size().wimage()+i;
      auto upperleft_gridsquare=GridCoordinate(i,j);
      auto viewport_pixel_0_grid=GridCoordinate(viewport_left_grid,viewport_top_grid);
      auto new_viewport_pixel_size=ViewportPixelSize(this->_image_max_size.wpixel(),this->_image_max_size.hpixel());
      auto viewport_pixel_coordinate=ViewportPixelCoordinate(upperleft_gridsquare,this->_zoom,viewport_pixel_0_grid,new_viewport_pixel_size);
      // TODO: this is where I chose zoom
      auto actual_zoom=zoom_index;
      // for testing max zoom
      // int actual_zoom=texture_grid->textures_max_zoom_index-1;
      auto lock_succeeded=false;
      auto try_loaded=true;
      bool texture_loaded=false;
      do {
        lock_succeeded=false;
        texture_loaded=false;
        // go through loop again
        if (try_loaded && !(actual_zoom <= max_zoom_index)) {
          actual_zoom=zoom_index;
          try_loaded=false;
        }
        auto texture_square_zoom=texture_grid->squares[i][j]->texture_array[actual_zoom].get();
        std::unique_lock<std::mutex> texture_square_lock(texture_square_zoom->display_mutex,std::defer_lock);
        if ((try_loaded &&
             texture_square_zoom->is_loaded &&
             texture_square_zoom->is_displayable) ||
            (!try_loaded && texture_square_zoom->is_displayable)) {
          // mutex_vector.emplace_back(std::unique_lock<std::mutex>{texture_square_zoom->display_mutex,std::defer_lock});
          if (texture_square_lock.try_lock()) {
            lock_succeeded=true;
            if (texture_grid->squares[i][j]->texture_array[actual_zoom]->display_texture_wrapper()->is_valid() ||
                (texture_grid->squares[i][j]->texture_array[actual_zoom]->get_image_filler() &&
                 texture_grid->squares[i][j]->texture_array[actual_zoom]->filler_texture_wrapper()->is_valid())) {
              texture_loaded=true;
              auto grid_image_size_zoomed=ViewportPixelSize((int)round(this->_image_max_size.wpixel()*this->_zoom),
                                                            (int)round(this->_image_max_size.hpixel()*this->_zoom));
              auto new_blit_item=std::make_unique<BlitItem>(texture_square_zoom,
                                                            gi,
                                                            viewport_pixel_coordinate,
                                                            grid_image_size_zoomed);
              blititems.push_back(std::move(new_blit_item));
            } else {
              texture_loaded=false;
            }
          }
        }
        // TODO: else raise error if things are terrible
        actual_zoom++;
      } while ((actual_zoom <= max_zoom_index || try_loaded) && (!lock_succeeded || !texture_loaded));
      blit_count++;
    }
  }
  // blit blitables
  auto viewport_pixel_size=ViewportPixelSize(this->_current_window_w,this->_current_window_h);
  std::unique_ptr<SDLDrawableSurface> drawable_surface=std::make_unique<SDLDrawableSurface>(sdl_app,viewport_pixel_size);
  for (size_t i=0; i < blititems.size(); i++) {
    blititems[i]->blit_this(drawable_surface.get());
  }
}

bool ViewPort::do_input(SDLApp* const sdl_app) {
  auto xgrid=this->_viewport_grid.xgrid();
  auto ygrid=this->_viewport_grid.ygrid();
  auto keep_going=sdl_app->do_input(this->_current_speed_x, this->_current_speed_y,
                                    this->_current_speed_zoom,this->_zoom, this->_zoom_speed,
                                    this->_image_max_size, xgrid, ygrid,
                                    this->_current_window_w,
                                    this->_current_window_h);
  this->update_viewport_info(xgrid,ygrid);
  return keep_going;
}

void ViewPort::update_viewport_info(FLOAT_T xgrid, FLOAT_T ygrid) {
  this->_viewport_grid=GridCoordinate(xgrid,ygrid);
  this->_viewport_pixel_size=ViewportPixelSize(this->_current_window_w,this->_current_window_h);
  // update the viewport
  this->_viewport_current_state_texturegrid_update->UpdateGridValues(this->_zoom,
                                                                     this->_viewport_grid,
                                                                     this->_image_max_size,
                                                                     this->_viewport_pixel_size);
  this->_viewport_current_state_imagegrid_update->UpdateGridValues(this->_zoom,
                                                                   this->_viewport_grid,
                                                                   this->_image_max_size,
                                                                   this->_viewport_pixel_size);

}

void ViewPort::adjust_initial_location(const GridSetup* const grid_setup) {
  // adjust initial position for small grids
  FLOAT_T new_xgrid,new_ygrid;
  if (grid_setup->grid_image_size().wimage() == 1) {
    new_xgrid=0.5;
  } else {
    new_xgrid=this->_viewport_grid.xgrid();
  }
  if (grid_setup->grid_image_size().himage() == 1) {
    new_ygrid=0.5;
  } else {
    new_ygrid=this->_viewport_grid.ygrid();
  }
  this->update_viewport_info(new_xgrid,new_ygrid);
}
