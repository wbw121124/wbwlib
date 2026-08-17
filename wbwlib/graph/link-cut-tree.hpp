#ifndef WBWLIB_GRAPH_LINK_CUT_TREE_HPP
#define WBWLIB_GRAPH_LINK_CUT_TREE_HPP

/**
 * @file link-cut-tree.hpp
 * @brief 动态树 LCT：link/cut/makeroot/findroot/路径和查询/路径加/点权修改。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * 摊还 O(log n)。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::graph::LinkCutTree<i64> lct(n);
 *   lct.link(u, v);                   // 加边（假定不成环）
 *   lct.cut(u, v);                    // 删 u-v 边
 *   lct.path_add(u, v, x);            // u→v 路径上点权 += x
 *   i64 s = lct.path_query(u, v);     // u→v 路径上点权和
 *   lct.set(x, v);                    // 单点赋权
 *   lct.makeroot(x); int r = lct.findroot(x);
 * @endcode
 */

#include <vector>
#include <algorithm>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace graph {

/**
 * @brief 动态树 LCT：支持 link/cut/makeroot/findroot、路径和查询、路径加、单点赋权。
 * @tparam T 点权类型
 */
template<class T>
class LinkCutTree {
  std::vector<int> fa_, ch0_, ch1_, rev_, siz_;
  std::vector<T> val_, sum_, add_;

  /// 判断 x 是否为所在 splay 的根（父节点不指向它）
  bool is_root(int x) const {
    return ch0_[fa_[x]] != x && ch1_[fa_[x]] != x;
  }
  /// 上传：由左右子树与自身重算 siz、sum
  void push_up(int x) {
    siz_[x] = siz_[ch0_[x]] + siz_[ch1_[x]] + 1;
    sum_[x] = sum_[ch0_[x]] + sum_[ch1_[x]] + val_[x];
  }
  /// 给 x 子树整体加 v（懒标记）
  void apply_add(int x, T v) {
    if (!x) return;
    val_[x] += v;
    sum_[x] += v * siz_[x];
    add_[x] += v;
  }
  /// 翻转 x 子树（懒标记）
  void apply_rev(int x) {
    if (!x) return;
    std::swap(ch0_[x], ch1_[x]);
    rev_[x] ^= 1;
  }
  /// 下传懒标记（翻转、加法）
  void push_down(int x) {
    if (rev_[x]) {
      apply_rev(ch0_[x]);
      apply_rev(ch1_[x]);
      rev_[x] = 0;
    }
    if (add_[x] != T()) {
      apply_add(ch0_[x], add_[x]);
      apply_add(ch1_[x], add_[x]);
      add_[x] = T();
    }
  }
  /// x 绕父节点 y 旋转（splay 基本操作）
  void rotate(int x) {
    int y = fa_[x], z = fa_[y];
    int k = (ch1_[y] == x);          // 1 表示 x 为 y 的右孩子
    if (!is_root(y)) {
      if (ch0_[z] == y) ch0_[z] = x;
      else ch1_[z] = x;
    }
    if (k) {
      ch1_[y] = ch0_[x];
      if (ch0_[x]) fa_[ch0_[x]] = y;
      ch0_[x] = y;
    } else {
      ch0_[y] = ch1_[x];
      if (ch1_[x]) fa_[ch1_[x]] = y;
      ch1_[x] = y;
    }
    fa_[y] = x;
    fa_[x] = z;
    push_up(y);
    push_up(x);
  }

  /**
   * @brief 构造 n 个孤立点的 LCT。
   * @param n 点数（1 基）
   */
 public:
  explicit LinkCutTree(int n) : fa_(n + 1, 0), ch0_(n + 1, 0), ch1_(n + 1, 0),
                               rev_(n + 1, 0), siz_(n + 1, 0),
                               val_(n + 1, T()), sum_(n + 1, T()), add_(n + 1, T()) {
    for (int i = 1; i <= n; ++i) siz_[i] = 1;
  }

  /**
   * @brief 把 x 伸展为其所在 splay 的根（先下推路径上标记，再双旋）。
   * @param x 节点（1 基）
   */
  void splay(int x) {
    std::vector<int> st;
    st.push_back(x);
    for (int y = x; !is_root(y); y = fa_[y]) st.push_back(fa_[y]);
    for (int i = (int)st.size() - 1; i >= 0; --i) push_down(st[i]);
    while (!is_root(x)) {
      int y = fa_[x], z = fa_[y];
      if (!is_root(y)) {
        bool same = ((ch1_[y] == x) == (ch1_[z] == y));
        if (same) rotate(y);
        else rotate(x);
      }
      rotate(x);
    }
  }

  /**
   * @brief 打通 x 到所在实树根节点的路径为一条实链，并把 x 伸展到 splay 根。
   * @param x 节点（1 基）
   */
  void access(int x) {
    int last = 0;
    for (int y = x; y; y = fa_[y]) {
      splay(y);
      ch1_[y] = last;
      push_up(y);
      last = y;
    }
    splay(x);
  }

  /**
   * @brief 把 x 变成其所在实树的根（access 后整棵 splay 打翻转标记）。
   * @param x 节点（1 基）
   */
  void makeroot(int x) {
    access(x);
    apply_rev(x);
  }

  /**
   * @brief 求 x 所在实树的根。
   * @param x 节点（1 基）
   * @return 树根节点编号
   */
  int findroot(int x) {
    access(x);
    while (ch0_[x]) {
      push_down(x);
      x = ch0_[x];
    }
    splay(x);
    return x;
  }

  /**
   * @brief 连接 u 与 v（假定不成环）。
   * @param u 节点（1 基）
   * @param v 节点（1 基）
   */
  void link(int u, int v) {
    makeroot(u);
    if (findroot(v) != u) fa_[u] = v;
  }

  /**
   * @brief 删除 u–v 边（假定存在且直接相连）。
   * @param u 节点（1 基）
   * @param v 节点（1 基）
   */
  void cut(int u, int v) {
    makeroot(u);
    access(v);
    if (ch0_[v] == u && ch1_[u] == 0) {
      ch0_[v] = 0;
      fa_[u] = 0;
      push_up(v);
    }
  }

  /**
   * @brief 把 u→v 路径抽到一棵 splay 上（splay 根为 v，u 为 v 左子树最左端），供查询/修改使用。
   * @param u 路径一端（1 基）
   * @param v 路径另一端（1 基）
   */
  void split(int u, int v) {
    makeroot(u);
    access(v);
    // 此时 splay 根为 v，u 为 v 左子树最左端
  }

  /**
   * @brief 查询 u→v 路径上的点权和。
   * @param u 路径一端（1 基）
   * @param v 路径另一端（1 基）
   * @return 路径点权和
   */
  T path_query(int u, int v) {
    split(u, v);
    return sum_[v];
  }

  /**
   * @brief u→v 路径上所有点权 += delta。
   * @param u 路径一端（1 基）
   * @param v 路径另一端（1 基）
   * @param delta 增量
   */
  void path_add(int u, int v, T delta) {
    split(u, v);
    apply_add(v, delta);
  }

  /**
   * @brief 单点赋权。
   * @param x 节点（1 基）
   * @param v 新点权
   */
  void set(int x, T v) {
    access(x);
    val_[x] = v;
    push_up(x);
  }
};

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_LINK_CUT_TREE_HPP