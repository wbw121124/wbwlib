#ifndef WBWLIB_DS_PERSISTENT_SEGTREE_HPP
#define WBWLIB_DS_PERSISTENT_SEGTREE_HPP

/**
 * @file persistent-segtree.hpp
 * @brief 可持久化线段树（主席树）：静态区间第 K 小/大、版本化点更新。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：O(log N) 每次更新/查询，额外 O(N log N) 结点。
 *
 * 用法（区间第 k 小）：
 *   vector<int> a = {1,5,2,6,3,7,4};
 *   wbwlib::ds::PersistentSegTree<int> st;
 *   st.build_discrete(a);           // 离散化
 *   for (int v : a) st.update_pos(st.roots.back(), ???)  // 见 update_by_value
 *
 * 典型操作：版本化点加（值域上累加计数）+ 两版本差值做第 k 小查询。
 *
 * 接口（rt 为版本根）：
 *   build(n)                       —— 建立空树索引范围 [1, n]
 *   update(prev, pos, delta)       —— 生成新版本（叶子加 delta），返回新根
 *   query_kth(rootL, rootR, k)     —— 求两版本值域差集内第 k 小的位置（1 基）
 *   query_count(rootL, rootR, ql, qr) —— 值域区间内元素个数
 */

#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

template<class T = i64>
class PersistentSegTree {
  struct Node { int l, r; T sum; };   // l, r 为左右孩子下标，-1 表示空
  std::vector<Node> pool_;
  int n_;

  int newnode() {
    pool_.push_back({-1, -1, T()});
    return (int)pool_.size() - 1;
  }

  int upd(int p, int l, int r, int pos, const T& delta) {
    int np = newnode();
    pool_[np] = pool_[p];
    if (l == r) {
      pool_[np].sum += delta;
      return np;
    }
    int mid = (l + r) >> 1;
    if (pos <= mid)
      pool_[np].l = upd(pool_[p].l, l, mid, pos, delta);
    else
      pool_[np].r = upd(pool_[p].r, mid + 1, r, pos, delta);
    pool_[np].sum = (pool_[np].l >= 0 ? pool_[pool_[np].l].sum : T()) +
                    (pool_[np].r >= 0 ? pool_[pool_[np].r].sum : T());
    return np;
  }

  int kth(int x, int y, int l, int r, int k) {   // x,y 两种版本同结构
    if (l == r) return l;
    int ll = pool_[x].l, lr = pool_[y].l;
    int left = (lr >= 0 ? pool_[lr].sum : T()) -
               (ll >= 0 ? pool_[ll].sum : T());
    T lc = left;                    // 差集左子树 count
    int mid = (l + r) >> 1;
    if (lc >= k) return kth(ll, lr, l, mid, k);
    return kth(pool_[x].r, pool_[y].r, mid + 1, r, k - lc);
  }

  T qcnt(int x, int y, int l, int r, int ql, int qr) {
    if (ql <= l && r <= qr)
      return T((y >= 0 ? pool_[y].sum : T()) - (x >= 0 ? pool_[x].sum : T()));
    int mid = (l + r) >> 1;
    T res = T();
    if (ql <= mid)
      res += qcnt(pool_[x].l, pool_[y].l, l, mid, ql, qr);
    if (qr > mid)
      res += qcnt(pool_[x].r, pool_[y].r, mid + 1, r, ql, qr);
    return res;
  }

 public:
  std::vector<int> roots;   ///< 版本根集合（roots[0] 为空树）

  /// 值域 [1, n]，roots[0] 为空版本
  void build(int n) {
    n_ = n;
    pool_.clear();
    pool_.push_back({-1, -1, T()});   // 0 号为空叶子
    roots.assign(1, 0);
  }

  /// 从 prev 版本生成新版本：在 pos（1 基）加 delta
  int update(int prev, int pos, const T& delta) {
    WBWLIB_ASSERT(prev >= 0 && pos >= 1 && pos <= n_);
    int rt = upd(prev, 1, n_, pos, delta);
    roots.push_back(rt);
    return rt;
  }

  /// 便捷：对当前最新版本做点更新，返回新根
  int update_latest(int pos, const T& delta) {
    return update(roots.back(), pos, delta);
  }

  /// x, y 两版本差值下的第 k 小位置（1 基，值域）
  int query_kth(int x, int y, int k) const {
    return kth(x, y, 1, n_, k);
  }

  /// 两版本差值下，值域 [ql, qr] 内元素个数
  T query_count(int x, int y, int ql, int qr) const {
    return qcnt(x, y, 1, n_, ql, qr);
  }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_PERSISTENT_SEGTREE_HPP