#ifndef WBWLIB_DS_LI_CHAO_TREE_HPP
#define WBWLIB_DS_LI_CHAO_TREE_HPP

/**
 * @file li-chao-tree.hpp
 * @brief 李超线段树：维护直线族 y = k*x + b 的最值查询。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * 插入/查询 O(log R)。支持在值域区间上插入直线（矩形极简版）。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::ds::LiChaoTree<i64, 1, 1000000> lct;      // x 值域 [1, 1e6]，求最大值
 *   lct.add_line(2, 3);        // y = 2x + 3
 *   i64 best = lct.query(4);   // max over lines at x=4
 * @endcode
 *
 * 求最小值：模板 Find 传 std::less，query 返回最小时的 k*x+b 值。
 */

#include <algorithm>
#include <functional>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

/**
 * @brief 李超线段树：维护直线族 \f$y = k \cdot x + b\f$ 的最值查询。
 *
 * 支持在值域 \f$[XLO, XHI]\f$ 上插入直线、单点查询最优值。
 * Cmp 取 std::greater 求最大值，取 std::less 求最小值。
 * @tparam T 系数与值的类型（默认 i64）
 * @tparam XLO 值域左端点（含）
 * @tparam XHI 值域右端点（含）
 * @tparam Cmp 最值比较器
 * @tparam PoolN 静态数组池大小（满二叉索引，需覆盖值域节点数）
 */
template<class T = i64, i64 XLO = 1, i64 XHI = 1000000,
         class Cmp = std::greater<void>, int PoolN = 1 << 20>
class LiChaoTree {
  struct Line { T k, b; bool has; };   ///< 直线 y = kx + b；has 表示该节点已插入直线
  std::vector<Line> seg_;      // 动态内存池（避免大数组占用栈）
  int cnt_ = 1;                // 已用节点数（根为 1）
  Cmp better_;

  /// 按 Cmp 语义比较两条直线在 x 处的取值
  bool better(const Line& l1, const Line& l2, i64 x) const {
    T v1 = l1.k * x + l1.b;
    T v2 = l2.k * x + l2.b;
    return v1 == v2 ? false : Cmp{}(v1, v2);
  }
  // 说明：better_ 实际按 Cmp 比较两"值"，此处用默认构造的 Cmp{}

 public:
  /**
   * @brief 构造李超树（节点池预分配 PoolN）。
   */
  LiChaoTree() : seg_(PoolN) {
    for (auto& s : seg_) s.has = false;
  }

  /**
   * @brief 插入直线 \f$y = kx + b\f$（覆盖整个值域 \f$[XLO, XHI]\f$）。
   * @param k 直线斜率
   * @param b 直线截距
   */
  void add_line(T k, T b) { add_line(k, b, XLO, XHI, 1); }

  /**
   * @brief 递归插入直线 \f$y = kx + b\f$ 到区间 [L, R]（动态开点）。
   * @param k 直线斜率
   * @param b 直线截距
   * @param L 插入区间左端点
   * @param R 插入区间右端点
   * @param id 当前节点下标
   */
  void add_line(T k, T b, i64 L, i64 R, int id) {
    Line nw{k, b, true};
    if (!seg_[id].has) {
      seg_[id] = nw;
      return;
    }
    i64 mid = (L + R) >> 1;
    bool lf = nw_lt(seg_[id], nw, L);
    bool md = nw_lt(seg_[id], nw, mid);
    if (md) /** @cond */ std::swap(seg_[id], nw) /** @endcond */;
    if (L == R) return;
    if (lf != md) {                       // 在左半相交
      add_line(nw.k, nw.b, L, mid, id << 1);
    } else {                              // 在右半相交
      add_line(nw.k, nw.b, mid + 1, R, id << 1 | 1);
    }
  }

  /**
   * @brief 查询 x 处所有直线的最优值（按 Cmp 语义）。
   * @param x 查询横坐标
   * @return x 处的最优 \f$k \cdot x + b\f$ 值
   */
  T query(i64 x) const { return query(x, XLO, XHI, 1); }

  /**
   * @brief 递归查询区间 [L, R] 上 x 的最优值。
   * @param x 查询横坐标
   * @param L 当前节点区间左端点
   * @param R 当前节点区间右端点
   * @param id 当前节点下标
   * @return x 处的最优值
   */
  T query(i64 x, i64 L, i64 R, int id) const {
    T res = 0;
    bool first = true;
    while (true) {
      if (seg_[id].has) {
        T v = seg_[id].k * x + seg_[id].b;
        if (first || good(res, v)) { res = v; first = false; }
      }
      if (L == R) break;
      i64 mid = (L + R) >> 1;
      if (x <= mid) { id = id << 1; R = mid; }
      else { id = id << 1 | 1; L = mid + 1; }
    }
    (void)better_;
    (void)first;
    return res;
  }

 private:
  /// nw 是否比已有直线更优（按 Cmp 语义在 x 处）
  bool nw_lt(const Line& a, const Line& b, i64 x) const {
    // nw 是否在已有行之上（按 Cmp 语义：greater 时 a<b 表示 nw 更优）
    return Cmp{}(b.k * x + b.b, a.k * x + a.b);
  }
  /// newv 是否优于 oldv（按 Cmp 语义）
  bool good(const T& oldv, const T& newv) const {
    return Cmp{}(newv, oldv);
  }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_LI_CHAO_TREE_HPP


