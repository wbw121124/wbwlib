#ifndef WBWLIB_GEO_CONVEX_HULL_3D_HPP
#define WBWLIB_GEO_CONVEX_HULL_3D_HPP

/**
 * @file convex-hull-3d.hpp
 * @brief 三维凸包：增量法 O(n²)（Luogu P4724【模板】三维凸包）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/core/random.hpp、wbwlib/geo/point3d.hpp
 *
 * @par 算法
 * 每次加入一个点 p：把能被 p「看见」（点在面法向量外侧）的三角形面删掉，
 * 保留看不见的面；对每条「明暗交界」边 (u,v)（vis[u][v]=1 且 vis[v][u]=0）
 * 新建三角形面 (u,v,p)。所有面定向为逆时针（法向量朝外）。
 * 面数 F ≤ 2V-4，复杂度 O(n²)。
 *
 * @par 注意
 * 为避免四点共面导致面片反复翻转，构造时默认对输入点做 ±1e-8 量级的随机扰动
 * （与 Luogu 模板题惯例一致；整数坐标请关闭扰动并自行保证无四点共面）。
 * 凸包为内部计算对象；可通过 faces() 取面片（顶点下标对应构造时传入的点序）。
 *
 * @par 示例
 * @code{.cpp}
 *   using namespace wbwlib::geo;
 *   std::vector<Point3D<i64>> pts = {{0,0,0},{1,0,0},{0,1,0},{0,0,1}};
 *   ConvexHull3D<i64> hull(pts);          // 默认扰动
 *   long double s = hull.surface_area();  // 正四面体: 2.3660254...
 * @endcode
 */

#include <vector>
#include <cmath>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/core/random.hpp"
#include "wbwlib/geo/point3d.hpp"

namespace wbwlib {
namespace geo {

/**
 * @brief 三维凸包（增量法）。
 * @tparam T 输入坐标类型（整数/浮点均可，内部统一用 long double 计算）。
 */
template<class T = i64>
class ConvexHull3D {
 public:
  /// 三角形面片：三个顶点下标 + 未归一化法向量。
  struct Face {
    int a, b, c;             ///< 顶点下标（逆时针，法向量朝外）
    Point3D<long double> n;  ///< 法向量 n = (b-a)×(c-a)

    /// 面片面积 = |n|/2。
    long double area() const { return n.norm() / 2; }
  };

  /**
   * @brief 由点集构造三维凸包。
   * @param pts   输入点集（会被复制，扰动不影响调用方）。
   * @param shake 是否对点做随机扰动（默认 true，防四点共面）。
   * @note 全部点共面/共线/重合时面数为 0，表面积 0。
   */
  explicit ConvexHull3D(const std::vector<Point3D<T>>& pts, bool shake = true) {
    points_.reserve(pts.size());
    if (shake) {
      for (const auto& p : pts)
        points_.push_back({(long double)p.x + jitter_(),
                           (long double)p.y + jitter_(),
                           (long double)p.z + jitter_()});
    } else {
      for (const auto& p : pts)
        points_.push_back({(long double)p.x, (long double)p.y, (long double)p.z});
    }
    build();
  }

  /**
   * @brief 凸包面片列表。
   * @return 面片数组（顶点下标指向内部排序去重后的点序）。
   */
  const std::vector<Face>& faces() const { return faces_; }

  /**
   * @brief 凸包面数（三角形面片总数）。
   * @return 面数。
   */
  size_t face_count() const { return faces_.size(); }

  /**
   * @brief 凸包表面积。
   * @return 各面片面积之和。
   */
  long double surface_area() const {
    long double s = 0;
    for (const auto& f : faces_) s += f.area();
    return s;
  }

 private:
  std::vector<Point3D<long double>> points_;
  std::vector<Face> faces_;

  /// 微小扰动 ±1e-8（全局随机源）。
  static long double jitter_() {
    return (long double)(core::rng.next() % 2000001) / 1e14 - 1e-8;
  }

  /// 构造面。
  static Face make_face_(int a, int b, int c, const std::vector<Point3D<long double>>& p) {
    return {a, b, c, (p[b] - p[a]).cross(p[c] - p[a])};
  }

  void build() {
    const long double kEps = 1e-9;
    // 排序去重（扰动后几乎不会去重，仅防整数关闭扰动时的重复点）
    std::vector<Point3D<long double>> pts = points_;
    std::sort(pts.begin(), pts.end());
    pts.erase(std::unique(pts.begin(), pts.end()), pts.end());
    points_.swap(pts);
    const int n = (int)points_.size();
    if (n < 4) return;

    // 找初始不共线三点
    int a = 0, b = -1, c = -1;
    for (int i = 1; i < n; ++i)
      if (!(points_[i] == points_[a])) { b = i; break; }
    if (b < 0) return;  // 全重合
    for (int i = b + 1; i < n; ++i) {
      if ((points_[b] - points_[a]).cross(points_[i] - points_[a]).norm2() > 1e-24) { c = i; break; }
    }
    if (c < 0) return;  // 全共线

    // 初始两个反向面，再根据其余点确定法向量朝外
    std::vector<Face> cur;
    cur.push_back(make_face_(a, b, c, points_));
    cur.push_back(make_face_(c, b, a, points_));
    for (int i = 0; i < n; ++i) {
      if (i == a || i == b || i == c) continue;
      if (std::fabs((points_[i] - points_[a]).dot(cur[0].n)) > 1e-9) {
        if ((points_[i] - points_[a]).dot(cur[0].n) < 0) std::swap(cur[0], cur[1]);
        break;
      }
    }
    // 若全部点共面，直接返回该平面（两个反向面面积相等，任取其一）
    bool planar = true;
    for (int i = 0; i < n && planar; ++i) {
      if (i == a || i == b || i == c) continue;
      if (std::fabs((points_[i] - points_[a]).dot(cur[0].n)) > 1e-9) planar = false;
    }
    if (planar) {
      faces_.push_back(cur[0]);
      return;
    }

    // 增量主循环
    std::vector<std::vector<char>> vis(n, std::vector<char>(n, 0));
    std::vector<Face> nxt;
    nxt.reserve(n * 4);
    for (int i = 0; i < n; ++i) {
      if (i == a || i == b || i == c) continue;
      nxt.clear();
      const Point3D<long double>& p = points_[i];
      for (const auto& f : cur) {
        bool up = (p - points_[f.a]).dot(f.n) > kEps;
        if (!up) nxt.push_back(f);
        vis[f.a][f.b] = vis[f.b][f.c] = vis[f.c][f.a] = (char)up;
      }
      for (const auto& f : cur) {
        if (vis[f.a][f.b] && !vis[f.b][f.a]) nxt.push_back(make_face_(f.a, f.b, i, points_));
        if (vis[f.b][f.c] && !vis[f.c][f.b]) nxt.push_back(make_face_(f.b, f.c, i, points_));
        if (vis[f.c][f.a] && !vis[f.a][f.c]) nxt.push_back(make_face_(f.c, f.a, i, points_));
      }
      cur.swap(nxt);
    }
    faces_.swap(cur);
  }
};

} // namespace geo
} // namespace wbwlib

#endif // WBWLIB_GEO_CONVEX_HULL_3D_HPP