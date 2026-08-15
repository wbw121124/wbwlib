#ifndef WBWLIB_DS_SEGTREE_HPP
#define WBWLIB_DS_SEGTREE_HPP

/**
 * @file segtree.hpp
 * @brief 线段树。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 提供两种：
 *  1. SegTree    —— 迭代线段树（单点修改 + 任意半群区间查询），O(log n)；
 *  2. LazySeg    —— 递归线段树：区间加/区间赋值 + 区间和/区间最大值，O(log n)。
 *
 * 用法（SegTree<long long>，op 可换为 max/min 等）：
 *   SegTree<long long> st(n, [](long long a, long long b){ return a+b; }, 0);
 *   st.update(pos, val); long long q = st.query(l, r);
 *
 * LazySeg 维护 sum 与 mx；
 *  - add(l,r,v) 区间加；
 *  - assign(l,r,v) 区间赋值为 v（覆盖旧加标记）；
 *  - sum(l,r) / max(l,r) 查询。
 */

#include <algorithm>
#include <functional>
#include <limits>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

// ================= 迭代线段树（点更新 + 区间查询） =================
template<class T, class Op = std::plus<T>>
class SegTree {
  int n_;
  std::vector<T> tr_;
  Op op_;
  T id_;

 public:
  SegTree() {}
  /**
   * @param n  元素个数（1 基）
   * @param op 半群运算（如 +, max）
   * @param id 单位元（如 0, -inf）
   */
  SegTree(int n, Op op = Op(), const T& id = T()) : n_(n), tr_(2 * n, id), op_(op), id_(id) {}

  /// 点更新：位置 p（1 基）设为 v
  void set(int p, const T& v) {
    p += n_ - 1;
    tr_[p] = v;
    for (p >>= 1; p; p >>= 1) tr_[p] = op_(tr_[p << 1], tr_[p << 1 | 1]);
  }

  /// 点更新：a[p] 与 v 合并（如 + 时等价于 +=）
  void update(int p, const T& v) { set(p, op_(tr_[p + n_ - 1], v)); }

  /// 区间查询 op_(a[l..r])，1 基闭区间
  T query(int l, int r) const {
    T resl = id_, resr = id_;
    for (l += n_ - 1, r += n_ - 1; l <= r; l >>= 1, r >>= 1) {
      if (l & 1) resl = op_(resl, tr_[l++]);
      if (!(r & 1)) resr = op_(tr_[r--], resr);
    }
    return op_(resl, resr);
  }

  T all() const { return tr_[1]; }

  /// 从 1 基数组 a 构造
  void build(const std::vector<T>& a) {
    for (int i = 0; i < n_; ++i) tr_[n_ + i] = a[i + 1];
    for (int i = n_ - 1; i >= 1; --i) tr_[i] = op_(tr_[i << 1], tr_[i << 1 | 1]);
  }
};

// ================= 递归线段树（区间加/赋值 + 区间和/最大值） =================
template<class T = i64, bool UseMax = true>
class LazySeg {
  struct Node {
    T sum, mx;
    T lz_add, lz_set;
    bool has_set;
    Node() : sum(T()), mx(T()), lz_add(T()), lz_set(T()), has_set(false) {}
  };
  int n_;
  std::vector<Node> t_;

  void apply_add(int p, int len, const T& v) {
    t_[p].sum += v * T(len);
    if (UseMax) t_[p].mx += v;
    t_[p].lz_add += v;
  }

  void apply_set(int p, int len, const T& v) {
    t_[p].sum = v * T(len);
    if (UseMax) t_[p].mx = v;
    t_[p].has_set = true;
    t_[p].lz_set = v;
    t_[p].lz_add = T();               // 赋值覆盖旧加法
  }

  void push(int p, int len) {
    if (len <= 1) return;
    int lc = p << 1, rc = p << 1 | 1;
    int hl = len >> 1, hr = len - hl;
    if (t_[p].has_set) {
      apply_set(lc, hl, t_[p].lz_set);
      apply_set(rc, hr, t_[p].lz_set);
      t_[p].has_set = false;
    }
    if (t_[p].lz_add != T()) {
      apply_add(lc, hl, t_[p].lz_add);
      apply_add(rc, hr, t_[p].lz_add);
      t_[p].lz_add = T();
    }
  }

  void pull(int p) {
    t_[p].sum = t_[p << 1].sum + t_[p << 1 | 1].sum;
    if (UseMax) t_[p].mx = (std::max)(t_[p << 1].mx, t_[p << 1 | 1].mx);
  }

  void build(int p, int l, int r, const std::vector<T>& a) {
    t_[p] = Node();
    if (l == r) { t_[p].sum = t_[p].mx = a[l]; return; }
    int mid = (l + r) >> 1;
    build(p << 1, l, mid, a);
    build(p << 1 | 1, mid + 1, r, a);
    pull(p);
  }

  void update(int p, int l, int r, int ql, int qr, const T& v, bool is_assign) {
    if (ql <= l && r <= qr) {
      if (is_assign) apply_set(p, r - l + 1, v);
      else apply_add(p, r - l + 1, v);
      return;
    }
    push(p, r - l + 1);
    int mid = (l + r) >> 1;
    if (ql <= mid) update(p << 1, l, mid, ql, qr, v, is_assign);
    if (qr > mid) update(p << 1 | 1, mid + 1, r, ql, qr, v, is_assign);
    pull(p);
  }

  T qsum(int p, int l, int r, int ql, int qr) {
    if (ql <= l && r <= qr) return t_[p].sum;
    push(p, r - l + 1);
    int mid = (l + r) >> 1;
    T res = T();
    if (ql <= mid) res += qsum(p << 1, l, mid, ql, qr);
    if (qr > mid) res += qsum(p << 1 | 1, mid + 1, r, ql, qr);
    return res;
  }

  T qmax(int p, int l, int r, int ql, int qr) {
    if (ql <= l && r <= qr) return t_[p].mx;
    push(p, r - l + 1);
    int mid = (l + r) >> 1;
    T res = T(-std::numeric_limits<i64>::max());
    if (ql <= mid) res = (std::max)(res, qmax(p << 1, l, mid, ql, qr));
    if (qr > mid) res = (std::max)(res, qmax(p << 1 | 1, mid + 1, r, ql, qr));
    return res;
  }

 public:
  explicit LazySeg(int n) : n_(n), t_(4 * n + 4) {}

  void build(const std::vector<T>& a) { build(1, 1, n_, a); }

  /// 区间加 a[l..r] += v
  void add(int l, int r, const T& v) { if (l <= r) update(1, 1, n_, l, r, v, false); }
  /// 区间赋值 a[l..r] = v
  void assign(int l, int r, const T& v) { if (l <= r) update(1, 1, n_, l, r, v, true); }
  /// 区间和
  T sum(int l, int r) {
    if (l > r) return T();
    return qsum(1, 1, n_, l, r);
  }
  /// 区间最大值（仅 UseMax=true 时有意义；T 为数值类型）
  T max(int l, int r) {
    static_assert(UseMax, "LazySeg<UseMax=false> 不支持 max 查询");
    if (l > r) return T(-std::numeric_limits<i64>::max());
    return qmax(1, 1, n_, l, r);
  }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_SEGTREE_HPP