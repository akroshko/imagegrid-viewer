/**
 * This manages the metadata that goes along with the image grid.
 */
#ifndef IMAGEGRID_METADATA_HPP
#define IMAGEGRID_METADATA_HPP
// local headers
#include "../common.hpp"
#include "../coordinates.hpp"
#include "imagegrid.hpp"
// C++ headers
#include <string>

/**
 * Stores the queried metadata.
 */
class MetadataInfo {
  // this could be a struct, but I expect a class will be  more appropriate
  // in the future
public:
  MetadataInfo();
  ~MetadataInfo()=default;
  /** The corresponding pixel coordinate of the queried metadata. */
  ImagePixelCoordinate pixel_coordinate;
  // TODO: haven't implemented real-world coordinates yet
  FLOAT64 real_world_x;
  FLOAT64 real_world_y;
};

class ImageGridMetadata {
public:
  ImageGridMetadata();
  ~ImageGridMetadata()=default;
  ImageGridMetadata(const ImageGridMetadata&)=delete;
  ImageGridMetadata(const ImageGridMetadata&&)=delete;
  ImageGridMetadata& operator=(const ImageGridMetadata&)=delete;
  ImageGridMetadata& operator=(const ImageGridMetadata&&)=delete;
  /**
   * Look up metadata.
   *
   * @param imagegrid The Image Grid to get the metadata from.
   * @param metadata_name Name of metadata to look up.
   * @param metadata_grid_coordinate Name of metadata to look up.
   * @param metadata_info This gets set with the requested metadata.
   */
  void get_metadata(const ImageGrid* const imagegrid,
                    std::string& metadata_name,
                    GridCoordinate& metadata_grid_coordinate,
                    MetadataInfo& metadata_info);
};
#endif
