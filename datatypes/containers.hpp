#ifndef CONTAINERS_HPP
#define CONTAINERS_HPP

#include "../common.hpp"
#include "coordinates.hpp"
// C++ headers
#include <memory>

// I don't actually use this so it's just empty
template <typename T>
class StaticArray {


};

/**
 * A simple array of unique pointers that is late constructed to a
 * fixed size that never changes after that.
 */
template <typename T>
class StaticArray<std::unique_ptr<T>> {
public:
  StaticArray()=default;
  ~StaticArray()=default;
  /**
   * Initialize this class with a particular array size.
   *
   * @param arr_size The size to initialize array to.
   */
  void init(const INT64 arr_size) {
    this->_arr_size=arr_size;
    this->_arr=std::make_unique<std::unique_ptr<T>[]>(arr_size);
  }
  // TODO: modifications will occur to use assignment operator
  /**
   * Set an array element.
   *
   * @param index The index to set.
   * @param value The value to set it to.
   */
  void set(INT64 index, std::unique_ptr<T> value) {
    this->_arr[index] = std::move(value);
  }
  /**
   * Access an array element.
   *
   * @param index The index to set.
   * @return The value of the array element.
   */
  T* operator[](const INT64 index) const{
    return this->_arr[index].get();
  }
protected:
  std::unique_ptr<std::unique_ptr<T>[]> _arr;
  INT64 _arr_size;
};

/**
 * A container for 2D grids that are late constructed with a size
 * that never changes after that.
 */
template <typename T>
class StaticGrid {
public:
  StaticGrid()=default;
  ~StaticGrid()=default;
  // TODO: modifications will occur to hide later initialization
  /**
   * Initialize this class with a particular grid size.
   *
   * @param grid_size The size to initialize grid to.
   */
  void init(const CoordinatePairINT<INT64>& grid_size) {
    this->_grid_size=grid_size;
    this->_grid=std::make_unique<T[]>(this->_grid_size._x1*this->_grid_size._x2);
  }
  // TODO: modifications will occur to use assignment operator
  /**
   * Set an grid element.
   *
   * @param index The index to set.
   * @param value The value to set it to.
   */
  void set(const CoordinatePairINT<INT64>& index, T value) {
    this->_grid[this->_grid_index(index)]=value;
  }
  /**
   * Access a grid element.
   *
   * @param index The index to set.
   * @return The value of the grid element.
   */
  T operator[](const CoordinatePairINT<INT64>& index) const {
    return this->_grid[this->_grid_index(index)];
  }
protected:
  INT64 _grid_index(const CoordinatePairINT<INT64>& grid_index) const {
    return grid_index._x2*this->_grid_size._x1+grid_index._x1;
  }
  std::unique_ptr<T[]> _grid;
  CoordinatePairINT<INT64> _grid_size;
};

// when the object in the template is a pointer
template <typename T>
class StaticGrid<std::unique_ptr<T>> {
public:
  StaticGrid()=default;
  ~StaticGrid()=default;
  // TODO: modifications will occur to hide later initialization
  /**
   * Initialize this class with a particular size.
   *
   * @param grid_size The size to initialize the grid to.
   */
  void init(const CoordinatePairINT<INT64>& grid_size) {
    this->_grid_size=grid_size;
    this->_grid=std::make_unique<std::unique_ptr<T>[]>(this->_grid_size._x1*this->_grid_size._x2);
  }

  // TODO: modifications will occur to use assignment operator
  /**
   * Set a grid element.
   *
   * @param index The index to set.
   * @param value The value to set it to.
   */
  void set(const CoordinatePairINT<INT64>& index, std::unique_ptr<T> value) {
    _grid[_grid_index(index)]=std::move(value);
  }
  /**
   * Access a grid element.
   *
   * @param index The index to set.
   * @return The value of the grid element.
   */
  T* operator[](const CoordinatePairINT<INT64>& index) const {
    return this->_grid[this->_grid_index(index)].get();
  }
protected:
  INT64 _grid_index(const CoordinatePairINT<INT64>& grid_index) const {
    return grid_index._x2*this->_grid_size._x1+grid_index._x1;
  }
  std::unique_ptr<std::unique_ptr<T>[]> _grid;
  CoordinatePairINT<INT64> _grid_size;
};

/**
 * A container for a 2D grid of 2D grids that are late constructed
 * with a size that never changes after that.
 */
template <typename T>
class StaticGridTwoLayer {
public:
  StaticGridTwoLayer()=default;
  ~StaticGridTwoLayer()=default;
  /**
   * Initialize the grid on the first layer.
   *
   * @param grid_size_layer_1 The size of the grid on the first layer.
   */
  void init_layer_1(const CoordinatePairINT<INT64>& grid_size_layer_1) {
    this->_grid_size_layer_1=grid_size_layer_1;
    this->_grid=std::make_unique<std::unique_ptr<T[]>[]>(grid_size_layer_1._x1*grid_size_layer_1._x2);
    this->_layer_2_sizes.init(grid_size_layer_1);
  }
  /**
   * Initialize a grid on the second layer corresponding to a grid
   * cell on the first layer.
   *
   * @param index_layer_1 The index of the grid cell on the first layer.
   * @param grid_size_layer_2 The size of the grid on the second layer.
   */
  void init_layer_2(const CoordinatePairINT<INT64>& index_layer_1, const CoordinatePairINT<INT64>& grid_size_layer_2) {
    auto layer_1_index_raw=index_layer_1._x2*this->_grid_size_layer_1._x1+index_layer_1._x1;
    this->_layer_2_sizes.set(index_layer_1,grid_size_layer_2);
    this->_grid[layer_1_index_raw]=std::make_unique<T[]>(grid_size_layer_2._x1*grid_size_layer_2._x2);
  }
  /**
   * Set the value of a grid value.
   *
   * @param index_layer_1 The index of the grid cell on the first layer.
   * @param index_layer_2 The index of the grid cell on the second layer.
   * @param The value to set it to.
   */
  void set(const CoordinatePairINT<INT64>& index_layer_1, const CoordinatePairINT<INT64>& index_layer_2, const T& value) {
    auto layer_1_index_raw=index_layer_1._x2*this->_grid_size_layer_1._x1+index_layer_1._x1;
    auto layer_2_size=this->_layer_2_sizes[index_layer_1];
    auto layer_2_index_raw=index_layer_2._x2*layer_2_size._x1+index_layer_2._x1;
    this->_grid[layer_1_index_raw][layer_2_index_raw]=value;
  }
  // TODO: maybe I should upgrade to C++ 2023 so I can have multiple operators on []
  /**
   * Access a grid element.
   *
   * @param index_layer_1 The index of the grid cell on the first layer.
   * @param index_layer_2 The index of the grid cell on the second layer.
   * @return The value of the grid element.
   */
  T operator()(const CoordinatePairINT<INT64>& index_layer_1, const CoordinatePairINT<INT64>& index_layer_2) const {
    auto layer_1_index_raw=index_layer_1._x2*this->_grid_size_layer_1._x1+index_layer_1._x1;
    auto layer_2_size=this->_layer_2_sizes[index_layer_1];
    auto layer_2_index_raw=index_layer_2._x2*layer_2_size._x1+index_layer_2._x1;
    return this->_grid[layer_1_index_raw][layer_2_index_raw];
  }
protected:
  std::unique_ptr<std::unique_ptr<T[]>[]> _grid;
  CoordinatePairINT<INT64> _grid_size_layer_1;
  StaticGrid<CoordinatePairINT<INT64>> _layer_2_sizes;
};

#endif
