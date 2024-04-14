// local headers
#include "../common.hpp"
#include "imagegrid_nts.hpp"
// C++ headers
#include <string>

NTSDataFromFile::NTSDataFromFile(std::string filename) {
  this->_abbreviation=filename;
}

std::string NTSDataFromFile::abbreviation () {
  return this->_abbreviation;
}
