#ifndef WBWLIB_GEO_POLAR_HPP
#define WBWLIB_GEO_POLAR_HPP

/**
 * @file polar.hpp
 * @brief 极角相关工具：按极角排序、角度转弧度、坐标→极角。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/geo/point.hpp
 */

#include <vector>
#include <algorithm>
#include <cmath>
#include "wbwlib/core/base.hpp"
#include "wbwlib/geo/point.hpp"

namespace wbwlib {
namespace geo {

/**
 * @brief 计算向量的极角。
 * @param v 二维向量。
 * @return 极角（弧度），\f$\operatorname{atan2}(v.y, v.x)\f$，范围 \f$[-\pi, \pi]\f$。
 */
inline long double angle_of(const PointD& v) {
  return std::atan2(v.y, v.x);
}

/**
 * @brief 把点集按相对原点的极角升序排序（double 版），同角按距离升序。
 * @param ps 待排序点集（原地修改）。
 */
inline void polar_sort(std::vector<PointD>& ps) {
  std::sort(ps.begin(), ps.end(), PolarAngleCmp<long double>());
}

/**
 * @brief 极角排序（整数版），同角按距离升序。
 * @param ps 待排序点集（原地修改）。
 */
inline void polar_sort(std::vector<PointI>& ps) {
  std::sort(ps.begin(), ps.end(), PolarAngleCmp<i64>());
}

/**
 * @brief 将角度归一化到 \f$[0, 2\pi)\f$。
 * @param a 输入角度（弧度）。
 * @return 与 a 等价的 \f$[0, 2\pi)\f$ 内角度。
 */
inline long double normalize_angle(long double a) {
  while (a < 0) a += 2 * acos((long double)-1);
  while (a >= 2 * acos((long double)-1)) a -= 2 * acos((long double)-1);
  return a;
}

/**
 * @brief 求一组向量两两之间的最小绝对夹角（弧度），用于点共圆扫描等。
 * @param ps 已按极角排序的向量集合。
 * @return 最小夹角（弧度）；向量数 < 2 时返回 0。
 */
inline long double min_angle_between_sorted(const std::vector<PointD>& ps) {
  int n = (int)ps.size();
  if (n < 2) return 0;
  long double best = (long double)1e30L;
  for (int i = 0; i < n; ++i) {
    long double a = angle_of(ps[(i + 1) % n]);
    long double b = angle_of(ps[i]);
    long double d = normalize_angle(a - b);   // [0,2π)
    best = std::min(best, std::min(d, (long double)2 * acos((long double)-1) - d));
  }
  return best;
}

} // namespace geo
} // namespace wbwlib

#endif // WBWLIB_GEO_POLAR_HPP