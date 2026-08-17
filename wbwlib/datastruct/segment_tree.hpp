#ifndef WBWLIB_DS_SEGMENT_TREE_HPP
#define WBWLIB_DS_SEGMENT_TREE_HPP

/**
 * @file segment_tree.hpp
 * @brief 泛型懒标记线段树（Tval 值类型 / Ttag 标记类型，策略全在模板参数）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * 模板参数（除 Tval/Ttag 外均为函数指针，默认是「区间加 + 区间和」）：
 *   Tval        节点聚合值类型（如 i64）
 *   Ttag        懒标记类型（如 i64）
 *   pushup(o,a,b)    由左右子聚合值 a,b 上拉节点 o（引用修改）
 *   addtag(o,t,l,r)  标记 t 作用到节点 o，l,r 为节点区间（可算区间长）
 *   hastag(t)        判断标记 t 非空（决定是否下传）
 *   merge(a,b)       查询时按序合并左右结果
 *   nullval()        查询单位元
 *   compose_tag(o,t) 新标记 t 复合进旧标记 o（签名 (Ttag&, const Ttag&)；
 *                    默认加法，赋值型标记应传覆盖 { o = t; }）
 *
 * 约定：Ttag() 是空标记（hastag(Ttag()) 须为 false）；操作均为 1 基闭区间。
 *
 * @par 示例
 * @code{.cpp}
 *   segment_tree<i64, i64> st(n);            // 区间加 + 区间和
 *   st.build(a);                             // a[1..n]
 *   st.range_apply(1, 3, 2);                 // a[1..3] += 2
 *   i64 s = st.query(2, 4);                  // 区间和
 * @endcode
 *
 *   // 区间最大值 + 区间加：
 *   static void mx_pushup(i64& o, const i64& a, const i64& b) { o = std::max(a, b); }
 *   static void mx_addtag(i64& o, const i64& t, int, int) { o += t; }
 *   static i64 mx_merge(i64 a, i64 b) { return std::max(a, b); }
 *   static i64 mx_null() { return -(1LL << 60); }
 *   segment_tree<i64, i64, mx_pushup, mx_addtag, segdetail::def_hastag<i64>,
 *                mx_merge, mx_null> st(n);
 *
 * @dot 懒标记下传示意（区间加后查询触发 push）
 * digraph pushdown {
 *   rankdir=TB; node [shape=box, style="rounded,filled", fillcolor="#fef3c7"];
 *   root [label="[1,4]  sum+=2\n标记 tag=2"];
 *   lc   [label="[1,2]  sum+=2\n标记 tag=2"];
 *   rc   [label="[3,4]  sum+=2\n标记 tag=2"];
 *   leaf [label="[1,1]  sum+=2"];
 *   root -> lc [label="push 下传", style=dashed];
 *   root -> rc [label="push 下传", style=dashed];
 *   lc -> leaf;
 * }
 * @enddot
 */

#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

namespace segdetail {

/// 默认 pushup：求和上拉
template<class V>
inline void def_pushup(V& o, const V& a, const V& b) { o = a + b; }
/// 默认 addtag：区间加（sum 语义，o += tag*(r-l+1)）
template<class V, class T>
inline void def_addtag(V& o, const T& t, int l, int r) {
  o += V(t) * V(r - l + 1);
}
/// 默认 hastag：tag != 0（数值型标记）
template<class T>
inline bool def_hastag(const T& t) { return t != T(); }
/// 默认 merge：求和
template<class V>
inline V def_merge(const V& a, const V& b) { return a + b; }
/// 默认 nullval：0
template<class V>
inline V def_nullval() { return V(); }
/// 默认 compose_tag：加法复合（区间加场景）
template<class T>
inline void def_compose(T& o, const T& t) { o = o + t; }

} // namespace segdetail

/**
 * @brief 泛型懒标记线段树（递归实现，4n 空间）：策略全部固化在模板参数中。
 *
 * 函数指针模板参数（pushup/addtag/hastag/merge/nullval/compose_tag）说明见文件头，
 * 默认组合为「区间加 + 区间和」。
 * @tparam Tval 节点聚合值类型（默认 i64）
 * @tparam Ttag 懒标记类型（默认 i64；约定 Ttag() 为空标记）
 */
template<class Tval = i64, class Ttag = i64,
         void (*pushup)(Tval&, const Tval&, const Tval&) = &segdetail::def_pushup<Tval>,
         void (*addtag)(Tval&, const Ttag&, int, int) = &segdetail::def_addtag<Tval, Ttag>,
         bool (*hastag)(const Ttag&) = &segdetail::def_hastag<Ttag>,
         Tval (*merge)(const Tval&, const Tval&) = &segdetail::def_merge<Tval>,
         Tval (*nullval)() = &segdetail::def_nullval<Tval>,
         void (*compose_tag)(Ttag&, const Ttag&) = &segdetail::def_compose<Ttag>>
class segment_tree {
  int n_;
  std::vector<Tval> val_;
  std::vector<Ttag> tag_;

  /// 把标记 t 应用到节点 p（改值 + 复合懒标记）
  void apply_node(int p, int l, int r, const Ttag& t) {
    addtag(val_[p], t, l, r);
    compose_tag(tag_[p], t);
  }

  /// 下传节点 p 的懒标记
  void push(int p, int l, int r) {
    if (l >= r || !hastag(tag_[p])) return;
    int mid = (l + r) >> 1;
    apply_node(p << 1, l, mid, tag_[p]);
    apply_node(p << 1 | 1, mid + 1, r, tag_[p]);
    tag_[p] = Ttag();
  }

  /// 用左右子值上拉节点 p
  void pull(int p) { pushup(val_[p], val_[p << 1], val_[p << 1 | 1]); }

  /// 递归建树（叶子取 a[l]）
  void build(int p, int l, int r, const std::vector<Tval>& a) {
    if (l == r) {
      val_[p] = a[l];
      return;
    }
    int mid = (l + r) >> 1;
    build(p << 1, l, mid, a);
    build(p << 1 | 1, mid + 1, r, a);
    pull(p);
  }

  /// 递归区间应用标记 t
  void apply(int p, int l, int r, int ql, int qr, const Ttag& t) {
    if (ql <= l && r <= qr) {
      apply_node(p, l, r, t);
      return;
    }
    push(p, l, r);
    int mid = (l + r) >> 1;
    if (ql <= mid) apply(p << 1, l, mid, ql, qr, t);
    if (qr > mid) apply(p << 1 | 1, mid + 1, r, ql, qr, t);
    pull(p);
  }

  /// 递归单点赋值（清空路径上的懒标记）
  void point_set(int p, int l, int r, int pos, const Tval& v) {
    if (l == r) {
      val_[p] = v;
      tag_[p] = Ttag();
      return;
    }
    push(p, l, r);
    int mid = (l + r) >> 1;
    if (pos <= mid) point_set(p << 1, l, mid, pos, v);
    else point_set(p << 1 | 1, mid + 1, r, pos, v);
    pull(p);
  }

  /// 递归区间查询（merge 按序合并结果）
  Tval query(int p, int l, int r, int ql, int qr) {
    if (ql <= l && r <= qr) return val_[p];
    push(p, l, r);
    int mid = (l + r) >> 1;
    Tval res = nullval();
    if (ql <= mid) res = merge(res, query(p << 1, l, mid, ql, qr));
    if (qr > mid) res = merge(res, query(p << 1 | 1, mid + 1, r, ql, qr));
    return res;
  }

 public:
  /**
   * @brief 构造覆盖 [1, n] 的线段树（元素初始为 Tval()）。
   * @param n 元素个数（1 基闭区间 [1, n]）
   */
  explicit segment_tree(int n) : n_(n), val_(4 * n + 4), tag_(4 * n + 4) {}

  /**
   * @brief 从 1 基数组 a 构造（仅使用 a[1..n]）。
   * @param a 1 基数组
   */
  void build(const std::vector<Tval>& a) {
    if (n_ >= 1) build(1, 1, n_, a);
  }

  /**
   * @brief 区间应用标记：\f$a[l..r]\f$ 处复合 t（如区间加）。
   * @param l 区间左端点（1 基闭区间）
   * @param r 区间右端点（1 基闭区间）
   * @param t 待应用的标记
   */
  void range_apply(int l, int r, const Ttag& t) {
    if (l <= r) apply(1, 1, n_, l, r, t);
  }

  /**
   * @brief 单点赋值：a[p] = v。
   * @param p 位置（1 基）
   * @param v 新值
   */
  void set(int p, const Tval& v) { point_set(1, 1, n_, p, v); }

  /**
   * @brief 区间查询：merge 按序合并 a[l..r] 的聚合值。
   * @param l 区间左端点（1 基闭区间）
   * @param r 区间右端点（1 基闭区间）
   * @return 聚合结果；l > r 时返回 nullval()
   */
  Tval query(int l, int r) {
    if (l > r) return nullval();
    return query(1, 1, n_, l, r);
  }

  /// 返回整棵树的聚合值（根节点）
  Tval all() const { return val_[1]; }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_SEGMENT_TREE_HPP