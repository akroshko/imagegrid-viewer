// local headers
#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "defaults.hpp"
#include "coordinates.hpp"
#include "viewport_current_state.hpp"
#include <cmath>
#include <mutex>

ViewPortCurrentState::ViewPortCurrentState () {
  this->zoom=NAN;
  this->zoom_last=NAN;
  this->been_updated=false;
}

void ViewPortCurrentState::UpdateGridValues(FLOAT_T zoom, const GridCoordinate &gridarg) {
  std::lock_guard<std::mutex> guard(this->using_mutex);
  if (std::isnan(this->zoom_last) || (this->grid_last.invalid()) || (zoom != this->zoom_last) || (gridarg.xgrid() != this->grid_last.xgrid()) || (gridarg.ygrid() != this->grid_last.ygrid())) {
    this->zoom=zoom;
    this->grid=GridCoordinate(gridarg);
    this->zoom_last=zoom;
    this->grid_last=GridCoordinate(gridarg);
    this->been_updated=true;
  }
}

bool ViewPortCurrentState::GetGridValues(FLOAT_T &zoom, GridCoordinate &gridarg) {
  std::lock_guard<std::mutex> guard(this->using_mutex);
  if (this->been_updated) {
    zoom=this->zoom;
    gridarg=GridCoordinate(this->grid);
    this->been_updated=false;
    return true;
  } else {
    return false;
  }
}
