#ifndef WBWLIB_GEO_HALF_PLANE_HPP
#define WBWLIB_GEO_HALF_PLANE_HPP

/**
 * @file half-plane.hpp
 * @brief 半平面交：极角排序 + 双端队列（实数版）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/geo/point.hpp
 *
 * @par 复杂度
 * O(n log n)。
 *
 * HalfPlane{ a, b } 语义：有向直线 a→b 左侧（含）为可行侧。
 * intersect() 返回可行多边形顶点（逆时针）；空返回空 vector。
 */

#include <vector>
#include <deque>
#include <cmath>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/geo/point.hpp"

namespace wbwlib {
namespace geo {

/**
 * @brief 半平面：有向直线 a→b 的左侧（含直线）为可行区域。
 */
struct HalfPlane {
  PointD a, b;      ///< 有向直线端点
  long double ang;  ///< 方向角 \f$\theta = \operatorname{atan2}(b_y-a_y, b_x-a_x)\f$
  /**
   * @brief 默认构造（成员未初始化）。
   */
  HalfPlane() {}
  /**
   * @brief 以有向直线两端点构造半平面。
   * @param a_ 起点。
   * @param b_ 终点，可行区域为 a→b 左侧（含直线）。
   */
  HalfPlane(const PointD& a_, const PointD& b_) : a(a_), b(b_) {
    ang = std::atan2(b.y - a.y, b.x - a.x);
  }
  /**
   * @brief 判断点是否在半平面内。
   * @param p 待判断点。
   * @return p 在直线 a→b 左侧或直线上（含容差）时返回 true。
   */
  bool contains(const PointD& p) const {
    return (b - a).cross(p - a) >= -1e-12L;
  }
};

/**
 * @brief 计算两条半平面边界直线的交点。
 * @param x 半平面一。
 * @param y 半平面二。
 * @return 交点坐标（调用方需保证两直线不平行）。
 */
inline PointD hp_intersect(const HalfPlane& x, const HalfPlane& y) {
  PointD u = x.b - x.a, v = y.b - y.a;
  long double den = u.cross(v);
  // den 不为 0（平行边已剔除）
  long double t = (y.a - x.a).cross(v) / den;
  return x.a + u * t;
}

/**
 * @brief 半平面交：求所有半平面可行区域的交集。
 * @param hps 半平面集合。
 * @return 交集多边形顶点（逆时针）；交为空或边界数不足 3 条（无界）时返回空 vector。
 */
inline std::vector<PointD> half_plane_intersection(std::vector<HalfPlane> hps) {
  // 极角增序；同角保留最左（最紧）：更严格的先排，去重时保留第一个
  std::sort(hps.begin(), hps.end(), [](const HalfPlane& A, const HalfPlane& B) {
    if (std::fabs(A.ang - B.ang) > 1e-12L) return A.ang < B.ang;
    return (A.b - A.a).cross(B.a - A.a) < 0;   // A 比 B 更靠左（更严格）
  });
  std::vector<HalfPlane> uniq;
  for (auto& hp : hps) {
    if (!uniq.empty() && std::fabs(hp.ang - uniq.back().ang) <= 1e-12L)
      continue;   // 同极角时 uniq 已保留更严格的
    uniq.push_back(hp);
  }
  std::deque<HalfPlane> q;
  std::deque<PointD> pts;      // q[i] 与 q[i+1] 的交点
  for (auto& hp : uniq) {
    while (q.size() >= 2 && !hp.contains(pts.back())) { q.pop_back(); pts.pop_back(); }
    while (q.size() >= 2 && !hp.contains(pts.front())) { q.pop_front(); pts.pop_front(); }
    if (!q.empty()) {
      long double den = (hp.b - hp.a).cross(q.back().b - q.back().a);
      if (std::fabs(den) <= 1e-12L) {
        // 平行：若反方向，无解
        if ((hp.b - hp.a).dot(q.back().b - q.back().a) < 0) return {};
        continue;
      }
      pts.push_back(hp_intersect(q.back(), hp));
    }
    q.push_back(hp);
  }
  while (q.size() >= 2 && !q.front().contains(pts.back())) { q.pop_back(); pts.pop_back(); }
  while (q.size() >= 2 && !q.back().contains(pts.front())) { q.pop_front(); pts.pop_front(); }
  if (q.size() < 3) return {};
  // 首尾交点
  std::vector<PointD> res;
  for (size_t i = 0; i + 1 < q.size(); ++i) res.push_back(pts[i]);
  res.push_back(hp_intersect(q.back(), q.front()));
  return res;
}

} // namespace geo
} // namespace wbwlib

#endif // WBWLIB_GEO_HALF_PLANE_HPP