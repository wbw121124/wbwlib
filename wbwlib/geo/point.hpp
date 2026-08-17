#ifndef WBWLIB_GEO_POINT_HPP
#define WBWLIB_GEO_POINT_HPP

/**
 * @file point.hpp
 * @brief 计算几何基础：二维点/向量，整数坐标（默认 i64）+ double 变体。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * 提供的运算符：+ - * /（标量）、点乘 dot、叉乘 cross、模长平方 norm2、
 * 比较、向量夹角工具、共线判断。
 */

#include <cmath>
#include <algorithm>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace geo {

/**
 * @brief 二维点/向量，整数与浮点坐标通用。
 * @tparam T 坐标类型，默认 i64（整数）；浮点场景显式指定 long double。
 */
template<class T = i64>
struct Point {
  T x, y;   ///< 坐标分量
  /**
   * @brief 默认构造，坐标零初始化。
   */
  Point() : x(), y() {}
  /**
   * @brief 以给定坐标构造。
   * @param x_ 横坐标。
   * @param y_ 纵坐标。
   */
  Point(T x_, T y_) : x(x_), y(y_) {}

  /**
   * @brief 向量加法。
   * @param o 另一向量。
   * @return 两向量之和。
   */
  Point operator+(const Point& o) const { return {x + o.x, y + o.y}; }
  /**
   * @brief 向量减法。
   * @param o 另一向量。
   * @return 两向量之差。
   */
  Point operator-(const Point& o) const { return {x - o.x, y - o.y}; }
  /**
   * @brief 数乘。
   * @param k 标量。
   * @return 各分量乘以 k。
   */
  Point operator*(T k) const { return {x * k, y * k}; }
  /**
   * @brief 数除。
   * @param k 标量（非零）。
   * @return 各分量除以 k。
   */
  Point operator/(T k) const { return {x / k, y / k}; }
  /**
   * @brief 相等比较。
   * @param o 另一向量。
   * @return 各分量均相等时返回 true。
   */
  bool operator==(const Point& o) const { return x == o.x && y == o.y; }
  /**
   * @brief 字典序比较（先横坐标后纵坐标）。
   * @param o 另一向量。
   * @return x 较小，或 x 相等时 y 较小，则返回 true。
   */
  bool operator<(const Point& o) const {
    return x != o.x ? x < o.x : y < o.y;
  }

  /**
   * @brief 点乘（内积）。
   * @param o 另一向量。
   * @return \f$x \cdot o.x + y \cdot o.y\f$。
   */
  T dot(const Point& o) const { return x * o.x + y * o.y; }
  /**
   * @brief 叉积（二维标量形式）。
   * @param o 另一向量。
   * @return \f$x \cdot o.y - y \cdot o.x\f$（大于 0 表示 o 在逆时针方向）。
   */
  T cross(const Point& o) const { return x * o.y - y * o.x; }
  /**
   * @brief 模长平方。
   * @return \f$x^2 + y^2\f$。
   */
  T norm2() const { return x * x + y * y; }
  /**
   * @brief 模长（欧氏长度）。
   * @return \f$\sqrt{x^2 + y^2}\f$。
   */
  long double len() const { return std::sqrt((long double)norm2()); }
};

/**
 * @brief 向量叉积（自由函数版）。
 * @tparam T 坐标类型。
 * @param a 左向量。
 * @param b 右向量。
 * @return a × b 的二维标量。
 */
template<class T>
inline T cross(const Point<T>& a, const Point<T>& b) { return a.cross(b); }
/**
 * @brief 向量点乘（自由函数版）。
 * @tparam T 坐标类型。
 * @param a 左向量。
 * @param b 右向量。
 * @return a · b。
 */
template<class T>
inline T dot(const Point<T>& a, const Point<T>& b) { return a.dot(b); }

typedef Point<i64> PointI;
typedef Point<long double> PointD;

/**
 * @brief 三点叉积的方向性判断。
 * @tparam T 坐标类型。
 * @param a 起点。
 * @param b 中间点。
 * @param c 终点。
 * @return \f$(b-a) \times (c-a)\f$：>0 逆时针（c 在 ab 左侧），<0 顺时针（右侧），=0 共线。
 */
template<class T>
inline T orient(const Point<T>& a, const Point<T>& b, const Point<T>& c) {
  return (b - a).cross(c - a);
}

/**
 * @brief 判断三点是否共线。
 * @tparam T 坐标类型。
 * @param a 点一。
 * @param b 点二。
 * @param c 点三。
 * @return 三点共线（叉积为零）时返回 true。
 */
template<class T>
inline bool collinear(const Point<T>& a, const Point<T>& b, const Point<T>& c) {
  return (b - a).cross(c - a) == 0;
}

/**
 * @brief 两点间距离的平方。
 * @tparam T 坐标类型。
 * @param a 点一。
 * @param b 点二。
 * @return \f$|a - b|^2\f$。
 */
template<class T>
inline T dist2(const Point<T>& a, const Point<T>& b) { return (a - b).norm2(); }

/**
 * @brief 极角排序比较器：按相对中心 (ox, oy) 的幅角 \f$\theta \in [-\pi, \pi)\f$ 升序，同角按距离升序。
 * @tparam T 坐标类型。
 */
template<class T>
struct PolarAngleCmp {
  T ox, oy;   ///< 排序中心坐标
  /**
   * @brief 构造比较器。
   * @param ox_ 排序中心横坐标。
   * @param oy_ 排序中心纵坐标。
   */
  PolarAngleCmp(T ox_ = 0, T oy_ = 0) : ox(ox_), oy(oy_) {}
  /**
   * @brief 比较 a 是否应排在 b 之前。
   * @param a 左操作数。
   * @param b 右操作数。
   * @return a 的极角小于 b 时返回 true；同角时距离中心较近者优先。
   */
  bool operator()(const Point<T>& a, const Point<T>& b) const {
    Point<T> A = a - Point<T>(ox, oy), B = b - Point<T>(ox, oy);
    bool ha = (A.y > 0 || (A.y == 0 && A.x >= 0));
    bool hb = (B.y > 0 || (B.y == 0 && B.x >= 0));
    if (ha != hb) return ha;
    T c = A.cross(B);
    if (c != 0) return c > 0;
    return A.norm2() < B.norm2();
  }
};

} // namespace geo
} // namespace wbwlib

#endif // WBWLIB_GEO_POINT_HPP