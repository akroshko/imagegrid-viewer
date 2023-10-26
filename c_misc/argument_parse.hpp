#include "../common.hpp"
// C++ headers
#include <string>
#include <vector>

/**
 * Parse standard arguments from command line.
 *
 * @param argc The argc parameter from main.
 * @param argv The argv parameter from main.
 * @param wimage Reference to update with the width of the image grid.
 * @param himage Reference to update with the height of the image grid.
 * @param write_cache Reference to update on whether to write the cache.
 * @param use_cache Reference to update on whether to use the cache.
 * @param successful Reference to update on whether arguments were valid.
 * @param path_value A path namesto search for images.
 * @param filenames A list of filenames.
 * @param text_filename A filename corresponding to a text file that
 *        contains the parameters for the imagegrid.
 */
void parse_standard_arguments(int argc, char* const* argv,
                              INT64& wimage, INT64& himage,
                              bool& write_cache, bool& use_cache, bool& successful,
                              std::string& path_value, std::vector<std::string>& filenames,
                              std::string& text_filename);
