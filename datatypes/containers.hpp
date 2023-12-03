#ifndef CONTAINERS_HPP
#define CONTAINERS_HPP

#include "../common.hpp"
#include "coordinates.hpp"
// C++ headers
#include <memory>

/**
 * A container for arrays that are late constructed with a size and
 * never changed.
 */
template <typename T>
class StaticArray {
public:
  StaticArray()=default;
  ~StaticArray()=default;
  // TODO: modifications will occur to hide later initialization
  /**
   * Initialize this class with a particular size.
   *
   * @param arr_size The size to initialize array to.
   */
  void init(const CoordinatePairINT<INT64>& arr_size);
  // TODO: modifications will occur to hide assignment
  /**
   * Set an array element.
   *
   * @param index The index to set.
   * @param value The value to set it to.
   */
  void set(const CoordinatePairINT<INT64>& index, T value);
  /**
   * Access an array element.
   * @param index The index to set.
   * @return The value of the array element.
   */
  T operator[](const CoordinatePairINT<INT64>& index) const;
protected:
  INT64 _arr_index(const CoordinatePairINT<INT64>& arr_index) const;
  std::unique_ptr<T[]> _arr;
  CoordinatePairINT<INT64> _arr_size;
};

template <typename T>
void StaticArray<T>::init(const CoordinatePairINT<INT64>& arr_size) {
  this->_arr_size=arr_size;
  this->_arr=std::make_unique<T[]>(this->_arr_size._x1*this->_arr_size._x2);
}

template <typename T>
void StaticArray<T>::set(const CoordinatePairINT<INT64>& index, T value) {
  _arr[_arr_index(index)]=value;
}

template <typename T>
T StaticArray<T>::operator[](const CoordinatePairINT<INT64>& index) const {
  return this->_arr[this->_arr_index(index)];
}

template <typename T>
INT64 StaticArray<T>::_arr_index(const CoordinatePairINT<INT64>& arr_index) const {
  return arr_index._x2*this->_arr_size._x1+arr_index._x1;
}

#endif
