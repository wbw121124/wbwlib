#ifndef WBWLIB_DS_LI_CHAO_TREE_HPP
#define WBWLIB_DS_LI_CHAO_TREE_HPP

/**
 * @file li-chao-tree.hpp
 * @brief 李超线段树：维护直线族 y = k*x + b 的最值查询。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：插入/查询 O(log R)。支持在值域区间上插入直线（矩形极简版）。
 *
 * 用法：
 *   wbwlib::ds::LiChaoTree<i64, 1, 1000000> lct;      // x 值域 [1, 1e6]，求最大值
 *   lct.add_line(2, 3);        // y = 2x + 3
 *   i64 best = lct.query(4);   // max over lines at x=4
 *
 * 求最小值：模板 Find 传 std::less，query 返回最小时的 k*x+b 值。
 */

#include <algorithm>
#include <functional>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

template<class T = i64, i64 XLO = 1, i64 XHI = 1000000,
         class Cmp = std::greater<void>, int PoolN = 1 << 20>
class LiChaoTree {
  struct Line { T k, b; bool has; };
  Line seg_[PoolN];          // 静态内存池
  int cnt_ = 1;              // 已用节点数（根为 1）
  Cmp better_;

  bool better(const Line& l1, const Line& l2, i64 x) const {
    T v1 = l1.k * x + l1.b;
    T v2 = l2.k * x + l2.b;
    return v1 == v2 ? false : Cmp{}()(v1, v2);
  }
  // 说明：better_ 实际按 Cmp 比较两"值"，此处用默认构造的 Cmp{}

 public:
  LiChaoTree() { seg_[0].has = false; seg_[1].has = false; }

  void add_line(T k, T b) { add_line(k, b, XLO, XHI, 1); }

  void add_line(T k, T b, i64 L, i64 R, int id) {
    Line nw{k, b, true};
    if (!seg_[id].has) {
      seg_[id] = nw;
      return;
    }
    i64 mid = (L + R) >> 1;
    bool lf = nw_lt(seg_[id], nw, L);
    bool md = nw_lt(seg_[id], nw, mid);
    if (md) std::swap(seg_[id], nw);
    if (L == R) return;
    if (lf != md) {                       // 在左半相交
      add_line(nw.k, nw.b, L, mid, id << 1);
    } else {                              // 在右半相交
      add_line(nw.k, nw.b, mid + 1, R, id << 1 | 1);
    }
  }

  T query(i64 x) const { return query(x, XLO, XHI, 1); }

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
  bool nw_lt(const Line& a, const Line& b, i64 x) const {
    // nw 是否在已有行之上（按 Cmp 语义：greater 时 a<b 表示 nw 更优）
    return Cmp{}()(b.k * x + b.b, a.k * x + a.b);
  }
  bool good(const T& oldv, const T& newv) const {
    return Cmp{}()(newv, oldv);
  }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_LI_CHAO_TREE_HPP