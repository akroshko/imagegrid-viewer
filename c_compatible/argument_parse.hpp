#include "../debug.hpp"
#include "../error.hpp"
#include "../types.hpp"
// C++ headers
#include <string>
#include <vector>

/**
 * Parse standard arguments from command line.
 *
 * @param argc The argc parameter from main.
 *
 * @param argv The argv parameter from main.
 *
 * @param wimage Reference to update with the width of the image grid.
 *
 * @param himage Reference to update with the height of the image grid.
 *
 * @param do_cache Reference to update on whether to create the cache.
 *
 * @param use_cache Reference to update on whether to use the cache.
 *
 * @param successful Reference to update on whether arguments were valid.
 *
 * @param path_value A path namesto search for images.
 *
 * @param filenames A list of filenames.
 */
void parse_standard_arguments(int argc, char* const* argv,
                              INT_T &wimage, INT_T &himage,
                              bool &do_cache, bool &use_cache, bool &successful,
                              std::string &path_value, std::vector<std::string> &filenames);
