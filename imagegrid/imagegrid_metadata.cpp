// local headers
#include "../common.hpp"
#include "../coordinates.hpp"
#include "imagegrid.hpp"
#include "imagegrid_metadata.hpp"
// C++ headers
#include <string>

MetadataInfo:: MetadataInfo () {

}

ImageGridMetadata::ImageGridMetadata () {

}

void ImageGridMetadata::get_metadata(const ImageGrid* const imagegrid,
                                     std::string& metadata_name,
                                     GridCoordinate& metadata_grid_coordinate,
                                     MetadataInfo& metadata_info) {
  if (metadata_name == "pixel_only") {
    // mirror incoming data for now
    auto max_pixel_size=imagegrid->get_image_max_pixel_size();
    auto image_pixel_coordinate=ImagePixelCoordinate(metadata_grid_coordinate,
                                                     max_pixel_size);
    metadata_info.pixel_coordinate=ImagePixelCoordinate(image_pixel_coordinate.x(),
                                                        image_pixel_coordinate.y());
    // TODO: no real world coordinates implemented for now
    metadata_info.real_world_x=0;
    metadata_info.real_world_y=0;
  }
}
