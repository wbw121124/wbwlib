#ifndef WBWLIB_DS_PERSISTENT_SEGTREE_HPP
#define WBWLIB_DS_PERSISTENT_SEGTREE_HPP

/**
 * @file persistent-segtree.hpp
 * @brief 可持久化线段树（主席树）：静态区间第 K 小/大、版本化点更新。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * O(log N) 每次更新/查询，额外 O(N log N) 结点。
 *
 * 用法（区间第 k 小）：
 *   vector<int> a = {1,5,2,6,3,7,4};
 *   wbwlib::ds::PersistentSegTree<int> st;
 *   st.build(7);                     // 值域 [1, 7]
 *   for (int v : a) st.update_latest(v, 1);
 *   st.query_kth(st.roots[1], st.roots[5], 2);   // 区间 [2,5] 第 2 小
 *
 * 典型操作：版本化点加（值域上累加计数）+ 两版本差值做第 k 小查询。
 *
 * 接口（rt 为版本根）：
 *   build(n)                       —— 建立空树索引范围 [1, n]
 *   update(prev, pos, delta)       —— 生成新版本（叶子加 delta），返回新根
 *   query_kth(rootL, rootR, k)     —— 求两版本值域差集内第 k 小的位置（1 基）
 *   query_count(rootL, rootR, ql, qr) —— 值域区间内元素个数
 *
 * 说明：节点池用 std::deque —— push_back 不移动元素也不释放旧块，
 *       下标（节点编号）永久有效，可安全地跨重分配持有。
 *
 * @dot 版本链示意（每次 update 沿路径新建结点）
 * digraph pst {
 *   rankdir=LR; node [shape=box, style="rounded,filled", fillcolor="#dcfce7"];
 *   v0 [label="版本0 根"];
 *   v1 [label="版本1 根"]; v2 [label="版本2 根"];
 *   n1 [label="新结点"]; n2 [label="新结点"];
 *   v0 -> v1 [label="update(1)"];
 *   v1 -> v2 [label="update(2)"];
 *   v1 -> n1 [style=dashed, label="复制路径"];
 *   v2 -> n2 [style=dashed, label="复制路径"];
 * }
 * @enddot
 */

#include <deque>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

/**
 * @brief 可持久化线段树（主席树）：版本化点更新 + 两版本差值的区间查询。
 *
 * 每次 update 生成新版本根；两版本对应位置的差即历史区间内元素的累计变化，
 * 典型用于静态区间第 k 小/大。
 * @tparam T 节点聚合值类型（默认 i64），需支持 += 与 -=。
 */
template<class T = i64>
class PersistentSegTree {
  struct Node { int l, r; T sum; };   // l, r 为左右孩子下标，-1 表示空
  std::deque<Node> pool_;
  int n_;

  /// 新建节点（0 号恒为空叶子）
  int newnode() {
    if (pool_.empty()) pool_.push_back({-1, -1, T()});   // 0 号空叶子
    pool_.push_back({-1, -1, T()});
    return (int)pool_.size() - 1;
  }

  /// 递归建新版本：p 的拷贝在 pos 处加 delta
  int upd(int p, int l, int r, int pos, const T& delta) {
    int np = newnode();
    if (p >= 0) pool_[np] = pool_[p];          // p 为 -1 表示空节点
    if (l == r) {
      pool_[np].sum += delta;
      return np;
    }
    int mid = (l + r) >> 1;
    if (pos <= mid)
      pool_[np].l = upd(p >= 0 ? pool_[p].l : -1, l, mid, pos, delta);
    else
      pool_[np].r = upd(p >= 0 ? pool_[p].r : -1, mid + 1, r, pos, delta);
    pool_[np].sum = (pool_[np].l >= 0 ? pool_[pool_[np].l].sum : T()) +
                    (pool_[np].r >= 0 ? pool_[pool_[np].r].sum : T());
    return np;
  }

  /// 递归求两版本差集内第 k 小的值域位置（x, y 同结构）
  int kth(int x, int y, int l, int r, int k) const {   // x,y 两种版本同结构
    if (l == r) return l;
    int ll = (x >= 0 ? pool_[x].l : -1), lr = (y >= 0 ? pool_[y].l : -1);
    int left = (lr >= 0 ? pool_[lr].sum : T()) -
               (ll >= 0 ? pool_[ll].sum : T());
    T lc = left;                    // 差集左子树 count
    int mid = (l + r) >> 1;
    if (lc >= k) return kth(ll, lr, l, mid, k);
    return kth(x >= 0 ? pool_[x].r : -1, y >= 0 ? pool_[y].r : -1,
               mid + 1, r, k - lc);
  }

  /// 递归求两版本差集在值域 [ql, qr] 内的元素个数
  T qcnt(int x, int y, int l, int r, int ql, int qr) const {
    if (ql <= l && r <= qr)
      return T((y >= 0 ? pool_[y].sum : T()) - (x >= 0 ? pool_[x].sum : T()));
    int mid = (l + r) >> 1;
    T res = T();
    if (ql <= mid)
      res += qcnt(x >= 0 ? pool_[x].l : -1, y >= 0 ? pool_[y].l : -1,
                  l, mid, ql, qr);
    if (qr > mid)
      res += qcnt(x >= 0 ? pool_[x].r : -1, y >= 0 ? pool_[y].r : -1,
                  mid + 1, r, ql, qr);
    return res;
  }

 public:
  std::deque<int> roots;   ///< 版本根集合（roots[0] 为空树）

  /**
   * @brief 建立值域 [1, n] 的空树，roots 重置为仅含空版本。
   * @param n 值域上界
   */
  void build(int n) {
    n_ = n;
    pool_.clear();
    pool_.push_back({-1, -1, T()});   // 0 号为空叶子
    roots.assign(1, 0);
  }

  /**
   * @brief 从 prev 版本生成新版本：在 pos 处加 delta（累加语义）。
   * @param prev 上一版本根（roots 中下标）
   * @param pos 值域位置（1 基，须在 [1, n]）
   * @param delta 增量
   * @return 新版本根，并自动追加到 roots
   */
  int update(int prev, int pos, const T& delta) {
    WBWLIB_ASSERT(prev >= 0 && pos >= 1 && pos <= n_);
    int rt = upd(prev, 1, n_, pos, delta);
    roots.push_back(rt);
    return rt;
  }

  /**
   * @brief 便捷：对当前最新版本做点更新。
   * @param pos 值域位置（1 基）
   * @param delta 增量
   * @return 新版本根
   */
  int update_latest(int pos, const T& delta) {
    return update(roots.back(), pos, delta);
  }

  /**
   * @brief 求两版本差集内的第 k 小值域位置（1 基）。
   * @param x 较早版本根
   * @param y 较晚版本根
   * @param k 名次（1 基）
   * @return 值域位置（1 基，[1, n]）
   */
  int query_kth(int x, int y, int k) const {
    return kth(x, y, 1, n_, k);
  }

  /**
   * @brief 求两版本差集内、值域 [ql, qr] 中的元素个数。
   * @param x 较早版本根
   * @param y 较晚版本根
   * @param ql 值域左端点（1 基）
   * @param qr 值域右端点（1 基）
   * @return 元素个数
   */
  T query_count(int x, int y, int ql, int qr) const {
    return qcnt(x, y, 1, n_, ql, qr);
  }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_PERSISTENT_SEGTREE_HPP