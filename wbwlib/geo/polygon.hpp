#ifndef WBWLIB_GEO_POLYGON_HPP
#define WBWLIB_GEO_POLYGON_HPP

/**
 * @file polygon.hpp
 * @brief 多边形工具：有向面积/法面积、点包含、周长、凸多边形判断。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/geo/point.hpp
 *
 * 说明：多边形按顶点顺序给出（逆时针为正）。面积类函数返回
 * 「二倍的有向面积」避免浮点（整数坐标）。
 */

#include <vector>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/geo/point.hpp"

namespace wbwlib {
namespace geo {

/**
 * @brief 计算多边形二倍的有向面积（避免浮点）。
 * @tparam T 坐标类型。
 * @param p 多边形顶点（按序给出，逆时针为正方向）。
 * @return \f$S_2 = \sum_{i} p_i \times p_{i+1}\f$，逆时针为正。
 */
template<class T>
T signed_area2(const std::vector<Point<T>>& p) {
  int n = (int)p.size();
  T s = 0;
  for (int i = 0; i < n; ++i) s += p[i].cross(p[(i + 1) % n]);
  return s;
}

/**
 * @brief 计算多边形有向面积。
 * @tparam T 坐标类型。
 * @param p 多边形顶点（按序给出）。
 * @return 有向面积 \f$S = \frac{1}{2}\sum_{i}(x_i y_{i+1} - x_{i+1} y_i)\f$，逆时针为正。
 */
template<class T>
long double area(const std::vector<Point<T>>& p) {
  return (long double)signed_area2(p) * 0.5L;
}

/**
 * @brief 计算多边形周长。
 * @tparam T 坐标类型。
 * @param p 多边形顶点（按序给出）。
 * @return 所有边边长之和。
 */
template<class T>
long double perimeter(const std::vector<Point<T>>& p) {
  int n = (int)p.size();
  long double res = 0;
  for (int i = 0; i < n; ++i)
    res += (p[i] - p[(i + 1) % n]).len();
  return res;
}

/**
 * @brief 判断点与多边形的位置关系（射线法 + 边界检测）。
 * @tparam T 坐标类型。
 * @param p 多边形顶点（按序给出）。
 * @param q 待判断点。
 * @return 0=外部，1=内部，2=边界上。
 */
template<class T>
int point_in_polygon(const std::vector<Point<T>>& p, const Point<T>& q) {
  int n = (int)p.size();
  bool inside = false;
  for (int i = 0, j = n - 1; i < n; j = i++) {
    // 边界测试
    if (orient(p[j], p[i], q) == 0 &&
        q.x >= std::min(p[i].x, p[j].x) && q.x <= std::max(p[i].x, p[j].x) &&
        q.y >= std::min(p[i].y, p[j].y) && q.y <= std::max(p[i].y, p[j].y))
      return 2;
    bool a = p[i].y > q.y, b = p[j].y > q.y;
    if (a != b) {
      T t = (q.y - p[j].y) * (p[i].x - p[j].x) - (q.x - p[j].x) * (p[i].y - p[j].y);
      if ((a && t < 0) || (!a && t > 0)) inside = !inside;
    }
  }
  return inside ? 1 : 0;
}

/**
 * @brief 判断多边形是否为凸多边形（顶点按序给出）。
 * @tparam T 坐标类型。
 * @param p 多边形顶点（按序给出）。
 * @return 顶点数 >= 3 且所有相邻三点的转向一致（共线忽略）时返回 true。
 */
template<class T>
bool is_convex_polygon(const std::vector<Point<T>>& p) {
  int n = (int)p.size();
  if (n < 3) return false;
  bool sign = false;
  for (int i = 0; i < n; ++i) {
    T o = orient(p[i], p[(i + 1) % n], p[(i + 2) % n]);
    if (o == 0) continue;
    bool cur = o > 0;
    if (!sign) sign = cur;
    else if (cur != sign) return false;
  }
  return true;
}

} // namespace geo
} // namespace wbwlib

#endif // WBWLIB_GEO_POLYGON_HPP