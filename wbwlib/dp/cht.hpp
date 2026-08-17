#ifndef WBWLIB_DP_CHT_HPP
#define WBWLIB_DP_CHT_HPP

/**
 * @file cht.hpp
 * @brief 凸包优化（CHT）：单调队列版 + 动态 multiset 版。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * 复杂度：
 *   MonoCHT     ：斜率/查询均单调，插入摊还 O(1)，查询摊还 O(1)。
 *   LineContainer：任意插入/查询，单次 O(log n)。
 *
 * 用法（求最小）：
 *   // 单调情形：
 *   wbwlib::dp::MonoCHT cht;                 // 默认求 min
 *   cht.add(-2*a[i], dpv);                   // y = m x + b
 *   i64 best = cht.query(x);
 *
 *   // 动态情形（内部按 max 维护，min 通过取负实现）：
 *   wbwlib::dp::LineContainer lc;
 *   lc.add_line(-m, -b);                     // 插入 y=mx+b 求 min 的等价写法
 *   i64 val = lc.query(-x);                  // 或显式用 addMin/queryMin
 *   lc.addMin(m, b);  i64 v = lc.queryMin(x);
 */

#include <vector>
#include <set>
#include <algorithm>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace dp {

/**
 * @brief 斜率单调递增、查询 x 单调不减的最小值 CHT（双端队列维护）。
 *
 * 对插入的直线 \f$y = mx + b\f$ 维护下凸壳，插入与查询均为摊还 O(1)。
 */
class MonoCHT {
  /// 一条直线 y = m*x + b
  struct Line { i64 m, b; };
  std::vector<Line> hull;   ///< 下凸壳上保留的直线
  int head;                 ///< 队首指针（head 之前的直线已过时）

  /// 交点比较：a.m < b.m < c.m，(b.b-a.b)/(a.m-b.m) >= (c.b-b.b)/(b.m-c.m) 时 b 冗余
  static bool bad(const Line& a, const Line& b, const Line& c) {
    return (b.b - a.b) * (b.m - c.m) >= (c.b - b.b) * (a.m - b.m);
  }

 public:
  /**
   * @brief 构造空 CHT。
   */
  MonoCHT() : head(0) {}

  /**
   * @brief 清空所有已插入的直线。
   */
  void clear() { hull.clear(); head = 0; }

  /**
   * @brief 插入直线 \f$y = m x + b\f$。
   * @param m 直线斜率，要求随调用单调不降。
   * @param b 直线截距。
   */
  void add(i64 m, i64 b) {
    Line ln{m, b};
    while ((int)hull.size() - head >= 2 &&
           bad(hull[hull.size() - 2], hull.back(), ln))
      hull.pop_back();
    hull.push_back(ln);
  }

  /**
   * @brief 查询所有已插入直线在 x 处的最小值。
   * @param x 查询横坐标，要求随调用单调不减。
   * @return \f$\min_i(m_i x + b_i)\f$。
   */
  i64 query(i64 x) {
    while (head + 1 < (int)hull.size() &&
           hull[head].m * x + hull[head].b >=
               hull[head + 1].m * x + hull[head + 1].b)
      ++head;
    return hull[head].m * x + hull[head].b;
  }
};

/**
 * @brief 动态 CHT：内部按上凸壳维护「最大」，提供 max/min 两套接口（KACTL 风格）。
 *
 * 支持任意顺序的插入与查询，单次 O(log n)；求最小通过在插入与查询时取负实现。
 */
class LineContainer {
  /// 一条直线 y = k*x + m；p 为该直线成为最优的 x 区间下界（交点阈值）
  struct Line {
    mutable i64 k, m, p;
    bool operator<(const Line& o) const { return k < o.k; }
    bool operator<(i64 x) const { return p < x; }
  };
  using It = std::multiset<Line, std::less<>>::iterator;
  std::multiset<Line, std::less<>> st_;
  static constexpr i64 INF = (i64)4e18;   ///< 交点阈值上界（视为无穷）

  /// 向负无穷取整的除法（计算交点横坐标用）
  static i64 div_f(i64 a, i64 b) {
    return a / b - ((a ^ b) < 0 && a % b);
  }
  /// 计算直线 x 与 y 的交点阈值；若 x 已无存在意义返回 true
  bool isect(It x, It y) {
    if (y == st_.end()) { x->p = INF; return false; }
    if (x->k == y->k) x->p = x->m > y->m ? INF : -INF;
    else x->p = div_f(y->m - x->m, x->k - y->k);
    return x->p >= y->p;
  }

 public:
  /**
   * @brief 清空所有已插入的直线。
   */
  void clear() { st_.clear(); }

  /**
   * @brief 插入直线 \f$y = k x + m\f$（内部按最大值维护）。
   * @param k 直线斜率。
   * @param m 直线截距。
   */
  void add_line(i64 k, i64 m) {
    auto z = st_.insert({k, m, 0}), y = z++, x = y;
    while (isect(y, z)) z = st_.erase(z);
    if (x != st_.begin() && isect(--x, y)) isect(x, y = st_.erase(y));
    while ((y = x) != st_.begin() && (--x)->p >= y->p)
      isect(x, st_.erase(y));
  }

  /**
   * @brief 查询所有已插入直线在 x 处的最大值（空容器时行为未定义）。
   * @param x 查询横坐标。
   * @return \f$\max_i(k_i x + m_i)\f$。
   */
  i64 query(i64 x) {
    // lower_bound(x)：比较器为 Line::operator<(i64)（按交点 p），
    // 找到第一个 p >= x 的直线即最优线
    auto it = st_.lower_bound(x);
    if (it == st_.end()) it = std::prev(it);
    return it->k * x + it->m;
  }

  /**
   * @brief 以最小值模式插入直线 \f$y = k x + m\f$（内部对 k、m 取负后维护）。
   * @param k 直线斜率。
   * @param m 直线截距。
   */
  void add_min_mode(i64 k, i64 m) { add_line(-k, -m); }

  /**
   * @brief 以最小值模式查询。
   * @param x 查询横坐标。
   * @return 所有已插入直线在 x 处的最小值。
   */
  i64 query_min_mode(i64 x) { return -query(x); }
};

} // namespace dp
} // namespace wbwlib

#endif // WBWLIB_DP_CHT_HPP