#ifndef IMAGEGRID_NTS_HPP
#define IMAGEGRID_NTS_HPP

/**
 * Some specific functions in support of Canada's NTS (National
 * Topographic System) maps.
 *
 * These are written so they can become base classes for other types
 * of data.
 */
// local headers
#include "../common.hpp"
// C++ headers
#include <string>

/**
 * Right now this just uses regexes and the filename, but will
 * eventually use an informational file.
 */
class NTSDataFromFile {
public:
  NTSDataFromFile(std::string filename);
  std::string abbreviation();
private:
  std::string _abbreviation;
};
#endif
