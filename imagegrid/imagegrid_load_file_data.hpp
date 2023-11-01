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
  std::unique_ptr<PIXEL_RGBA*[]> rgba_data;
  std::unique_ptr<size_t[]> rgba_wpixel;
  std::unique_ptr<size_t[]> rgba_hpixel;
  INT64 max_sub_wpixel=INT_MIN;
  INT64 max_sub_hpixel=INT_MIN;
  INT64 zoom_out=INT_MIN;
};

/**
 * TODO: fill in later when done refactoring
 *
 */
class LoadFileDataTransfer {
public:
  LoadFileDataTransfer();
  std::vector<std::shared_ptr<LoadFileZoomLevelData>> data_transfer;
  INT64 sub_w=INT_MIN;
  INT64 sub_h=INT_MIN;
  std::unique_ptr<size_t[]> original_rgba_wpixel;
  std::unique_ptr<size_t[]> original_rgba_hpixel;
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
