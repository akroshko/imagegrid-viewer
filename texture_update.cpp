/**
 * Implementation of updating the loaded and zoomed textures.
 */
#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "utility.hpp"
#include "coordinates.hpp"
#include "gridclasses.hpp"
#include "texture_update.hpp"

TextureUpdate::TextureUpdate(ViewPortCurrentState *viewport_current_state_texturegrid_update) {
  this->viewport_current_state_texturegrid_update = viewport_current_state_texturegrid_update;
}

int TextureUpdate::find_zoom_index(FLOAT_T zoom) {
  return ::find_zoom_index(zoom);
}

void TextureUpdate::find_current_textures (ImageGrid *grid, TextureGrid *texture_grid) {
  FLOAT_T zoom;
  auto view_changed=this->viewport_current_state_texturegrid_update->GetGridValues(zoom,this->viewport_grid);
  DEBUG("TextureUpdate::find_current_textures()");
  // if (view_changed) {
  // don't do anything here if viewport_current_state hasn't been initialized
  if (!this->viewport_grid.invalid()) {
    auto max_wpixel=texture_grid->max_pixel_size.wpixel();
    auto max_hpixel=texture_grid->max_pixel_size.hpixel();
    auto max_zoom=texture_grid->textures_max_zoom_index;
    auto max_zoom_index=max_zoom-1;
    auto zoom_index=this->find_zoom_index(zoom);
    DEBUG("zoom: " << zoom << " zoom index 1: " << zoom_index << " max: " << texture_grid->textures_max_zoom_index);
    if (zoom_index > max_zoom_index) {
      zoom_index=max_zoom_index;
    }
    if (zoom_index < 0) {
      zoom_index=0;
    }
    DEBUG("zoom index 2: " << zoom_index);
    DEBUG("max_zoom: " << max_zoom);
    for (INT_T z = 0ul; z < max_zoom; z++) {
      // only load/update current zoom, current zoom+-1 and max_zoom
      // this keeps things smoother
      if (z != max_zoom_index && z != zoom_index-1 && z !=zoom_index && z != zoom_index+1 ) {
        continue;
      }
      // calculate these with max resolution, rather than actual viewport
      auto max_zoom_this_level=1.0/(2.0+z);
      auto half_width=(MAX_SCREEN_WIDTH/2);
      auto half_height=(MAX_SCREEN_HEIGHT/2);
      auto max_zoom_left=(this->viewport_grid.xgrid())-(half_width/max_wpixel/max_zoom_this_level);
      // auto max_zoom_right=(viewport_grid->xgrid())+(half_width/max_wpixel/max_zoom_this_level);
      auto max_zoom_top=(this->viewport_grid.ygrid())-(half_height/max_hpixel/max_zoom_this_level);
      // auto max_zoom_bottom=(viewport_grid->ygrid())+(half_height/max_hpixel/max_zoom_this_level);
      auto threshold=2.0;
      // find out if this is 3x3 or full
      // edge is less than 0.5*threshold away from center
      DEBUG("zoom level: " << z << " viewport_xgrid: " << this->viewport_grid.xgrid() << " viewport_ygrid: " << this->viewport_grid.ygrid()
            << " max_zoom_this_level: " << max_zoom_this_level << " max_zoom_left: "
            << max_zoom_left // << " max_zoom_right: " << max_zoom_right
            << " max_zoom_top: " << max_zoom_top // << " max_zoom_bottom: " << max_zoom_bottom
        );
      auto load_all=false;
      if (!(z == max_zoom_index) && (abs(max_zoom_left - this->viewport_grid.xgrid()) <= 0.5*threshold) && (abs(max_zoom_top - this->viewport_grid.ygrid()) <= 0.5*threshold)) {
        // can load a 3x3 grid
        load_all=false;
        DEBUG("Updating textures to 3x3 at zoom level: " << z);
      } else {
        // always load everthing for last zoom level
        // just make sure everything is loaded for this zoom level
        load_all=true;
        DEBUG("Updating textures to full at zoom level: " << z);
      }
      this->update_textures(grid,
                            texture_grid,
                            z,
                            load_all);
    }
  } else {
    DEBUG("Skipping find_current_textures due to invalid viewport.");
  }
  DEBUG("TextureUpdate::find_current_textures() end");
  // }
}

// TODO: the grid showing what's going on doesn't work in cases right now
// TextureGrid::
void TextureUpdate::update_textures(ImageGrid *grid,
                                    TextureGrid *texture_grid,
                                    INT_T zoom_level,
                                    bool load_all) {
  DEBUG("update_textures() " << zoom_level << " | " << this->viewport_grid.xgrid() << " | " << this->viewport_grid.ygrid());
  // j,i is better for plotting a grid
  for (INT_T j = 0ul; j < texture_grid->grid_image_size.himage(); j++) {
    MSGNONEWLINE("| ");
    // TODO: this will eventually be a switch statement to load in different things
    for (INT_T i = 0ul; i < texture_grid->grid_image_size.wimage(); i++) {
      auto dest_square=texture_grid->squares[i][j].texture_array[zoom_level];
      // try and get the source square mutex
      if (!load_all &&
          ((i < next_smallest(this->viewport_grid.xgrid())) ||
           (i > next_largest(this->viewport_grid.xgrid())) ||
           (j < next_smallest(this->viewport_grid.ygrid())) ||
           (j > next_largest(this->viewport_grid.ygrid())))) {
        MSGNONEWLINE("0 ");
        // skip everything if locked
        std::unique_lock<std::mutex> display_lock(dest_square->display_mutex, std::defer_lock);
        if (display_lock.try_lock()) {
          if (dest_square->display_texture != nullptr) {
            SDL_FreeSurface(dest_square->display_texture);
            dest_square->display_texture = nullptr;
          }
          display_lock.unlock();
        } else {
          DEBUG("Unable to aquire display_mutex for deletion in TextureGrid::update_textures");
        }
      } else {
        auto load_index=IMAGE_GRID_BASE_INDEX;
        // auto load_index=1;
        auto image_square=grid->squares[i][j].image_array[load_index];
        if(image_square->is_loaded) {
          std::unique_lock<std::mutex> load_lock(image_square->load_mutex, std::defer_lock);
          if(load_lock.try_lock()) {
            MSGNONEWLINE("L ");
            std::unique_lock<std::mutex> display_lock(dest_square->display_mutex, std::defer_lock);
            if (display_lock.try_lock()) {
              texture_grid->squares[i][j].texture_pixel_size=GridPixelSize(texture_grid->max_pixel_size);
              if (dest_square->display_texture == nullptr) {
                this->load_texture(dest_square,
                                   image_square,
                                   zoom_level,
                                   texture_grid->squares[i][j].texture_pixel_size.wpixel(),
                                   texture_grid->squares[i][j].texture_pixel_size.hpixel());

              } else {
                DEBUG("Display texture is nullptr in TextureGrid::update_textures");
              }
              display_lock.unlock();
            } else {
              DEBUG("Unable to aquire display_mutex in TextureGrid::update_textures");
            }
            load_lock.unlock();
          } else {
            DEBUG("Unable to aquire load_mutex in TextureGrid::update_textures corresponding to: " << i << " " << j);
          }
        } else {
          DEBUG("File not loaded according to is_loaded in TextureGrid::update_textures: " << i << " " << j);
        }
      }
    }
    MSGNONEWLINE("|" << std::endl);
  }
  DEBUG("update_textures() end");
}

bool TextureUpdate::load_texture (TextureGridSquareZoomLevel *dest_square,
                                  ImageGridSquareZoomLevel *source_square,
                                  INT_T zoom_level,
                                  INT_T wpixel,
                                  INT_T hpixel) {
  auto successful=true;
  auto source_wpixel=(INT_T)source_square->rgb_wpixel;
  auto source_hpixel=(INT_T)source_square->rgb_hpixel;
  auto zoom_reduction=((INT_T)pow(2,zoom_level));
  auto dest_wpixel=wpixel/zoom_reduction;
  auto dest_hpixel=hpixel/zoom_reduction;
  dest_wpixel=dest_wpixel + (TEXTURE_ALIGNMENT - (dest_wpixel % TEXTURE_ALIGNMENT));
  // dest_wpixel=pad(dest_wpixel,TEXTURE_ALIGNMENT);
  dest_square->display_texture = SDL_CreateRGBSurfaceWithFormat(0,dest_wpixel,dest_hpixel,24,SDL_PIXELFORMAT_RGB24);
  // skip if can't load texture
  if (dest_square->display_texture) {
    auto lock_surface_return=SDL_LockSurface(dest_square->display_texture);
    if (lock_surface_return == 0) {
        auto skip = zoom_reduction;
        // does the thing we are copying exist?
        auto source_data=source_square->rgb_data;
        auto dest_array=dest_square->display_texture->pixels;
        if (dest_array != nullptr && source_data != nullptr) {
          // if (zoom_level == 0) {
          //   // use memcpy to hopefully take advantage of standard library when zoom index is zero
          //   for (INT_T l = 0ul; l < source_hpixel; l+=skip) {
          //     auto dest_index = (l*dest_wpixel)*3;
          //     auto source_index = (l*source_wpixel)*3;
          //     memcpy(((unsigned char *)dest_array)+dest_index,
          //            ((unsigned char *)source_data)+source_index,
          //            sizeof(unsigned char)*source_wpixel*3);
          //   }
          // } else {
          INT_T ld=0;
          for (INT_T l = 0ul; l < source_hpixel; l+=skip) {
            INT_T kd=0;
            for (INT_T k = 0ul; k < source_wpixel; k+=skip) {
              // auto source_index = (l*source_wpixel+k)*3;
              auto dest_index = (ld*dest_wpixel+kd)*3;
              INT_T sum_0=0;
              INT_T sum_1=0;
              INT_T sum_2=0;
              INT_T number_sum=0;
              // TODO: need proper memory alignment for this!!!
              for (auto ls = l; ls < l+skip; ls++) {
                for (auto ks = k; ks < k+skip; ks++) {
                  if (ls < source_hpixel && ks < source_wpixel) {
                    auto source_index = (ls*source_wpixel+ks)*3;
                    sum_0+=source_data[source_index];
                    sum_1+=source_data[source_index+1];
                    sum_2+=source_data[source_index+2];
                    number_sum+=1;
                  }
                }
              }
              ((unsigned char *)dest_array)[dest_index]=(unsigned char)round((FLOAT_T)sum_0/(FLOAT_T)number_sum);
              ((unsigned char *)dest_array)[dest_index+1]=(unsigned char)round((FLOAT_T)sum_1/(FLOAT_T)number_sum);
              ((unsigned char *)dest_array)[dest_index+2]=(unsigned char)round((FLOAT_T)sum_2/(FLOAT_T)number_sum);
              // ((unsigned char *)dest_array)[dest_index]=source_data[source_index];
              // ((unsigned char *)dest_array)[dest_index+1]=source_data[source_index+1];
              // ((unsigned char *)dest_array)[dest_index+2]=source_data[source_index+2];
              kd++;
            }
            ld++;
          }
        } else {
          DEBUG("Source or dest data is null in TextureGrid::load_texture");
        }
        SDL_UnlockSurface(dest_square->display_texture);
      } else {
        DEBUG("SDL_LockSurface failed with: " << lock_surface_return);
        dest_square->display_texture=nullptr;
        return false;
      }
  } else {
    DEBUG("SDL_CreateRGBSurfaceWithFormat failed with: " << SDL_GetError());
    return false;
  }
  return successful;
}
