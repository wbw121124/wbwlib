#ifndef WBWLIB_GEO_CIRCLE_HPP
#define WBWLIB_GEO_CIRCLE_HPP

/**
 * @file circle.hpp
 * @brief 圆：三点定圆、最小覆盖圆（随机增量）、点与圆位置、两圆关系。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/geo/point.hpp
 *
 * @par 复杂度
 * 三点定圆 O(1)；最小覆盖圆期望 O(n)。
 */

#include <vector>
#include <cmath>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/geo/point.hpp"

namespace wbwlib {
namespace geo {

/**
 * @brief 圆：圆心 + 半径。
 */
struct Circle {
  PointD c;         ///< 圆心
  long double r;    ///< 半径
  /**
   * @brief 默认构造（圆心原点，半径 0）。
   */
  Circle() : c(), r(0) {}
  /**
   * @brief 以圆心与半径构造圆。
   * @param c_ 圆心。
   * @param r_ 半径。
   */
  Circle(const PointD& c_, long double r_) : c(c_), r(r_) {}

  /**
   * @brief 判断点是否在圆内（含边界，带容差）。
   * @param p 待判断点。
   * @return \f$|p - c|^2 \le r^2\f$（含容差）时返回 true。
   */
  bool contains(const PointD& p) const {
    return (p - c).norm2() <= r * r + 1e-12L;
  }
};

/**
 * @brief 由三点确定外接圆。
 * @param a 点一。
 * @param b 点二。
 * @param c 点三。
 * @return 经过三点的圆（三点不共线时有效）。
 */
inline Circle circle_from_three_points(const PointD& a, const PointD& b, const PointD& c) {
  long double d = (a - c).cross(b - c);
  long double ax = (a.x - c.x), ay = (a.y - c.y);
  long double bx = (b.x - c.x), by = (b.y - c.y);
  long double A = ax * ax + ay * ay;
  long double B = bx * bx + by * by;
  long double u = (by * A - ay * B) / (2 * d);
  long double v = (ax * B - bx * A) / (2 * d);
  PointD center(u + c.x, v + c.y);
  return Circle(center, std::sqrt((center - a).norm2()));
}

/**
 * @brief 以两点为直径端点构造圆。
 * @param a 端点一。
 * @param b 端点二。
 * @return 以线段 ab 为直径的圆。
 */
inline Circle circle_from_two_points(const PointD& a, const PointD& b) {
  PointD mid((a.x + b.x) * 0.5L, (a.y + b.y) * 0.5L);
  return Circle(mid, std::sqrt((a - b).norm2()) * 0.5L);
}

/**
 * @brief 求点集的最小覆盖圆（随机增量法，期望 O(n)）。
 * @param ps 点集（按值传入，内部会伪随机洗牌）。
 * @return 覆盖所有点的最小半径圆；空集时返回圆心原点、半径 0 的圆。
 */
inline Circle min_enclosing_circle(std::vector<PointD> ps) {
  int n = (int)ps.size();
  if (n == 0) return Circle(PointD(), 0);
  if (n == 1) return Circle(ps[0], 0);
  // 伪随机洗牌（确定性，无需 <random>）：期望线性
  for (int i = n - 1; i > 0; --i) {
    std::swap(ps[i], ps[2654435761u * (unsigned)i % (i + 1)]);
  }
  Circle cur = circle_from_two_points(ps[0], ps[1]);
  for (int i = 2; i < n; ++i) {
    if (cur.contains(ps[i])) continue;
    cur = circle_from_two_points(ps[0], ps[i]);
    for (int j = 1; j < i; ++j) {
      if (cur.contains(ps[j])) continue;
      cur = circle_from_two_points(ps[i], ps[j]);
      for (int k = 0; k < j; ++k) {
        if (cur.contains(ps[k])) continue;
        cur = circle_from_three_points(ps[i], ps[j], ps[k]);
      }
    }
  }
  return cur;
}

/**
 * @brief 判断两圆的位置关系。
 * @param A 圆一。
 * @param B 圆二。
 * @return 按圆心距 \f$d = |A.c - B.c|\f$ 与半径和/差比较：
 *         0=相离 1=外切 2=相交 3=内切 4=内含 5=重合。
 */
inline int circle_relation(const Circle& A, const Circle& B) {
  long double d = std::sqrt((A.c - B.c).norm2());
  long double sum = A.r + B.r, dif = std::fabs(A.r - B.r);
  if (std::fabs(d) < 1e-12L && std::fabs(A.r - B.r) < 1e-12L) return 5;
  if (d > sum + 1e-12L) return 0;
  if (std::fabs(d - sum) <= 1e-12L) return 1;
  if (d > dif + 1e-12L) return 2;
  if (std::fabs(d - dif) <= 1e-12L) return 3;
  return 4;
}

} // namespace geo
} // namespace wbwlib

#endif // WBWLIB_GEO_CIRCLE_HPP