/**
 * Data structures for transfering imagegrid square data around.
 */
#include "../common.hpp"
// C++ headers
#include <memory>
#include <string>
#include <utility>
#include <vector>
// C headers
#include <climits>
#include <cstddef>

class ImageGridSquareZoomLevel;

/**
 * Contains loaded file data in preparation to be transferred to
 * ImageGridSquareZoomLevel.
 *
 * May not be permanent, members correspond to those in
 * ImageGridSquareZoomLevel.
 */
class LoadFileZoomLevelData {
public:
  LoadFileZoomLevelData();
  std::string filename;
  std::unique_ptr<std::unique_ptr<unsigned char *[]>[]> rgb_data;
  std::unique_ptr<std::unique_ptr<size_t[]>[]> rgb_wpixel;
  std::unique_ptr<std::unique_ptr<size_t[]>[]> rgb_hpixel;
  INT64 max_subgrid_wpixel=INT_MIN;
  INT64 max_subgrid_hpixel=INT_MIN;
  INT64 zoom_out_value=INT_MIN;
};

/**
 * TODO: fill in later when done refactoring
 *
 */
class LoadFileDataTransfer {
public:
  LoadFileDataTransfer();
  std::vector<std::shared_ptr<LoadFileZoomLevelData>> data_transfer;
  INT64 subgrid_w=INT_MIN;
  INT64 subgrid_h=INT_MIN;
  std::unique_ptr<std::unique_ptr<size_t[]>[]> original_rgb_wpixel;
  std::unique_ptr<std::unique_ptr<size_t[]>[]> original_rgb_hpixel;
};

/**
 * Contains loaded file data in preparation to be transferred to
 * multiple ImageGridSquareZoomLevel.
 */
class LoadFileData {
public:
  LoadFileData();
  std::vector<std::pair<ImageGridSquareZoomLevel* const,
                        std::shared_ptr<LoadFileZoomLevelData>>> data_pairs;
};
