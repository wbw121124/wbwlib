#ifndef WBWLIB_GEO_CONVEX_HULL_HPP
#define WBWLIB_GEO_CONVEX_HULL_HPP

/**
 * @file convex-hull.hpp
 * @brief 凸包：Andrew 单调链（整数坐标）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/geo/point.hpp
 *
 * @par 复杂度
 * O(n log n)。
 *
 * 返回：逆时针凸包顶点（首尾不重复）。若去掉共线以 bool 控制，
 * keep_collinear=false 时只保留凸包严格拐点。
 */

#include <vector>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/geo/point.hpp"

namespace wbwlib {
namespace geo {

/**
 * @brief 求点集凸包（Andrew 单调链），返回逆时针凸包顶点（首尾不重复）。
 * @tparam T 坐标类型（整数）。
 * @param ps 点集（按值传入，内部会排序与去重）。
 * @param keep_collinear true 时保留边上的共线点（边方向从左下到右上），false 时只保留严格拐点。
 * @return 逆时针凸包顶点；空集/一点/两点时返回去重后的点集。
 */
template<class T>
std::vector<Point<T>> convex_hull(std::vector<Point<T>> ps, bool keep_collinear = false) {
  if (ps.size() <= 1) return ps;
  std::sort(ps.begin(), ps.end());
  ps.erase(std::unique(ps.begin(), ps.end()), ps.end());
  if (ps.size() == 1) return ps;
  int n = (int)ps.size();
  std::vector<Point<T>> hull;
  hull.reserve(2 * n);
  // 下凸壳
  for (int i = 0; i < n; ++i) {
    while ((int)hull.size() >= 2) {
      T o = orient(hull[hull.size() - 2], hull.back(), ps[i]);
      if (o > 0 || (keep_collinear && o == 0)) break;   // 左转或保留共线
      hull.pop_back();
    }
    hull.push_back(ps[i]);
  }
  // 上凸壳
  int lower = (int)hull.size();
  for (int i = n - 2; i >= 0; --i) {
    while ((int)hull.size() > lower) {
      T o = orient(hull[hull.size() - 2], hull.back(), ps[i]);
      if (o > 0 || (keep_collinear && o == 0)) break;
      hull.pop_back();
    }
    hull.push_back(ps[i]);
  }
  hull.pop_back();   // 移除起点重复（首尾相接）
  // 若所有点共线，上面会退化为一条链；hull 仍有效（含两端点）。
  return hull;
}

/**
 * @brief 判断点是否在凸包内（含边界），凸包须按逆时针给出（二分角度法）。
 *
 * 依次判断叉积 \f$\operatorname{orient}(h_0, h_m, p)\f$ 的符号进行二分。
 *
 * @tparam T 坐标类型。
 * @param hull 逆时针凸包顶点。
 * @param p 待判断点。
 * @return p 在凸包内或边界上时返回 true。
 */
template<class T>
bool point_in_convex(const std::vector<Point<T>>& hull, const Point<T>& p) {
  int n = (int)hull.size();
  if (n == 0) return false;
  if (n == 1) return p == hull[0];
  if (n == 2) return collinear(hull[0], hull[1], p) &&
                       p.x >= std::min(hull[0].x, hull[1].x) &&
                       p.x <= std::max(hull[0].x, hull[1].x) &&
                       p.y >= std::min(hull[0].y, hull[1].y) &&
                       p.y <= std::max(hull[0].y, hull[1].y);
  if (orient(hull[0], hull[1], p) < 0) return false;
  if (orient(hull[0], hull[n - 1], p) > 0) return false;
  // 二分找到最后一个 orient(hull[0], hull[mid], p) >= 0
  int l = 1, r = n - 1;
  while (r - l > 1) {
    int mid = (l + r) >> 1;
    if (orient(hull[0], hull[mid], p) >= 0) l = mid;
    else r = mid;
  }
  return orient(hull[l], hull[r], p) >= 0;
}

} // namespace geo
} // namespace wbwlib

#endif // WBWLIB_GEO_CONVEX_HULL_HPP