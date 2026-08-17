#ifndef WBWLIB_GEO_POINT3D_HPP
#define WBWLIB_GEO_POINT3D_HPP

/**
 * @file point3d.hpp
 * @brief 三维点/向量基础：整数坐标（默认 i64）+ 浮点变体。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * 提供的运算：+ - * /（标量）、点乘 dot、叉乘 cross、模长、比较。
 */

#include <cmath>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace geo {

/**
 * @brief 三维点/向量。
 * @tparam T 坐标类型，默认 i64；浮点场景显式指定 long double。
 */
template<class T = i64>
struct Point3D {
  T x, y, z;  ///< 坐标分量

  Point3D() : x(), y(), z() {}
  /**
   * @brief 以给定坐标构造。
   * @param x_ 横坐标。
   * @param y_ 纵坐标。
   * @param z_ 竖坐标。
   */
  Point3D(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}

  /// 向量加法。
  Point3D operator+(const Point3D& o) const { return {x + o.x, y + o.y, z + o.z}; }
  /// 向量减法。
  Point3D operator-(const Point3D& o) const { return {x - o.x, y - o.y, z - o.z}; }
  /// 数乘。
  Point3D operator*(T k) const { return {x * k, y * k, z * k}; }
  /// 数除。
  Point3D operator/(T k) const { return {x / k, y / k, z / k}; }
  /// 相等比较。
  bool operator==(const Point3D& o) const { return x == o.x && y == o.y && z == o.z; }
  /// 字典序比较（x 优先）。
  bool operator<(const Point3D& o) const {
    if (x != o.x) return x < o.x;
    if (y != o.y) return y < o.y;
    return z < o.z;
  }

  /**
   * @brief 点乘（内积）。
   * @param o 另一向量。
   * @return 各分量乘积之和。
   */
  T dot(const Point3D& o) const { return x * o.x + y * o.y + z * o.z; }
  /**
   * @brief 叉乘（外积）。
   * @param o 另一向量。
   * @return 垂直于两者的向量（右手系）。
   */
  Point3D cross(const Point3D& o) const {
    return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
  }
  /**
   * @brief 模长平方。
   * @return x²+y²+z²。
   */
  T norm2() const { return x * x + y * y + z * z; }
  /**
   * @brief 模长。
   * @return sqrt(x²+y²+z²)。
   */
  long double norm() const { return std::sqrt((long double)norm2()); }
};

/**
 * @brief 以三个点构造三角形面积（用叉乘模长的一半）。
 * @tparam T 坐标类型。
 * @param a 三角形顶点。
 * @param b 三角形顶点。
 * @param c 三角形顶点。
 * @return 三角形面积（long double，防溢出）。
 */
template<class T>
long double triangle_area3d(const Point3D<T>& a, const Point3D<T>& b, const Point3D<T>& c) {
  return (b - a).cross(c - a).norm() / 2;
}

} // namespace geo
} // namespace wbwlib

#endif // WBWLIB_GEO_POINT3D_HPP