#ifndef WBWLIB_DS_SEGTREE_BEATS_HPP
#define WBWLIB_DS_SEGTREE_BEATS_HPP

/**
 * @file segtree-beats.hpp
 * @brief 吉司机线段树（Segment Tree Beats）：区间取 min / 区间加 / 区间和、区间最大值。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：均摊 O(log n log n)（取 min 的摊还分析）。
 *
 * 支持：
 *   add(l,r,v)        区间加
 *   chmin(l,r,x)      a[i] = min(a[i], x)（吉司机）
 *   sum(l,r)          区间和
 *   max(l,r)          区间最大值
 *   max2 / cnt_max    （可选内部对象，供强扩展使用）
 *
 * 用法：
 *   wbwlib::ds::SegBeats<i64> sb(n);
 *   sb.build(arr)； sb.chmin(1, n, 5)； sb.add(1, 3, 2)； sb.sum(2, 5)；
 */

#include <algorithm>
#include <limits>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

template<class T = i64>
class SegBeats {
  struct Node {
    T mx, mx2, sum;   ///< 最大、次大（严格）、和
    int cmx;          ///< 最大值的个数
    T lz;             ///< 加法懒标记
  };
  int n_;
  std::vector<Node> t_;
  T neg_inf_;

  void pull(int p) {
    Node &u = t_[p], &a = t_[p << 1], &b = t_[p << 1 | 1];
    u.sum = a.sum + b.sum;
    if (a.mx > b.mx) {
      u.mx = a.mx; u.cmx = a.cmx;
      u.mx2 = std::max(a.mx2, b.mx);
    } else if (a.mx < b.mx) {
      u.mx = b.mx; u.cmx = b.cmx;
      u.mx2 = std::max(a.mx, b.mx2);
    } else {
      u.mx = a.mx; u.cmx = a.cmx + b.cmx;
      u.mx2 = std::max(a.mx2, b.mx2);
    }
  }

  void apply_add(int p, int len, const T& v) {
    t_[p].mx += v;
    if (t_[p].mx2 != neg_inf_) t_[p].mx2 += v;
    t_[p].sum += v * T(len);
    t_[p].lz += v;
  }

  void apply_min(int p, int len, const T& x) {
    if (x >= t_[p].mx) return;
    t_[p].sum += (x - t_[p].mx) * T(t_[p].cmx);
    t_[p].mx = x;
  }

  void push(int p, int len) {
    if (len <= 1) return;
    if (t_[p].lz != T()) {
      apply_add(p << 1, len - (len >> 1), t_[p].lz);
      apply_add(p << 1 | 1, len >> 1, t_[p].lz);
      t_[p].lz = T();
    }
    apply_min(p << 1, len - (len >> 1), t_[p].mx);
    apply_min(p << 1 | 1, len >> 1, t_[p].mx);
  }

  void build(int p, int l, int r, const std::vector<T>& a) {
    t_[p] = Node();
    if (l == r) {
      t_[p].mx = t_[p].sum = a[l];
      t_[p].mx2 = neg_inf_;
      t_[p].cmx = 1;
      return;
    }
    int mid = (l + r) >> 1;
    build(p << 1, l, mid, a);
    build(p << 1 | 1, mid + 1, r, a);
    pull(p);
  }

  void add(int p, int l, int r, int ql, int qr, const T& v) {
    if (ql <= l && r <= qr) { apply_add(p, r - l + 1, v); return; }
    push(p, r - l + 1);
    int mid = (l + r) >> 1;
    if (ql <= mid) add(p << 1, l, mid, ql, qr, v);
    if (qr > mid) add(p << 1 | 1, mid + 1, r, ql, qr, v);
    pull(p);
  }

  void chmin(int p, int l, int r, int ql, int qr, const T& x) {
    if (x >= t_[p].mx) return;                 // 剪枝：全部无需改
    if (ql <= l && r <= qr && x > t_[p].mx2) { // 整体取 min 生效
      apply_min(p, r - l + 1, x);
      return;
    }
    push(p, r - l + 1);
    int mid = (l + r) >> 1;
    if (ql <= mid) chmin(p << 1, l, mid, ql, qr, x);
    if (qr > mid) chmin(p << 1 | 1, mid + 1, r, ql, qr, x);
    pull(p);
  }

  T qsum(int p, int l, int r, int ql, int qr) {
    if (ql <= l && r <= qr) return t_[p].sum;
    push(p, r - l + 1);
    int mid = (l + r) >> 1;
    T rs = T();
    if (ql <= mid) rs += qsum(p << 1, l, mid, ql, qr);
    if (qr > mid) rs += qsum(p << 1 | 1, mid + 1, r, ql, qr);
    return rs;
  }

  T qmax(int p, int l, int r, int ql, int qr) {
    if (ql <= l && r <= qr) return t_[p].mx;
    push(p, r - l + 1);
    int mid = (l + r) >> 1;
    T m = neg_inf_;
    if (ql <= mid) m = std::max(m, qmax(p << 1, l, mid, ql, qr));
    if (qr > mid) m = std::max(m, qmax(p << 1 | 1, mid + 1, r, ql, qr));
    return m;
  }

 public:
  explicit SegBeats(int n) : n_(n), t_(4 * n + 4),
                             neg_inf_(-std::numeric_limits<i64>::max()) {}

  void build(const std::vector<T>& a) { build(1, 1, n_, a); }

  void add(int l, int r, const T& v) { if (l <= r) add(1, 1, n_, l, r, v); }

  void chmin(int l, int r, const T& x) { if (l <= r) chmin(1, 1, n_, l, r, x); }

  T sum(int l, int r) { return (l <= r) ? qsum(1, 1, n_, l, r) : T(); }

  T max(int l, int r) {
    if (l > r) return neg_inf_;
    return qmax(1, 1, n_, l, r);
  }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_SEGTREE_BEATS_HPP