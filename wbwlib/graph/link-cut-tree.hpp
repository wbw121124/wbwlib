#ifndef WBWLIB_GRAPH_LINK_CUT_TREE_HPP
#define WBWLIB_GRAPH_LINK_CUT_TREE_HPP

/**
 * @file link-cut-tree.hpp
 * @brief 动态树 LCT：link/cut/makeroot/findroot/路径和查询/路径加/点权修改。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：摊还 O(log n)。
 *
 * 用法：
 *   wbwlib::graph::LinkCutTree<i64> lct(n);
 *   lct.link(u, v);                   // 加边（假定不成环）
 *   lct.cut(u, v);                    // 删 u-v 边
 *   lct.path_add(u, v, x);            // u→v 路径上点权 += x
 *   i64 s = lct.path_query(u, v);     // u→v 路径上点权和
 *   lct.set(x, v);                    // 单点赋权
 *   lct.makeroot(x); int r = lct.findroot(x);
 */

#include <vector>
#include <algorithm>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace graph {

template<class T>
class LinkCutTree {
  std::vector<int> fa_, ch0_, ch1_, rev_, siz_;
  std::vector<T> val_, sum_, add_;

  bool is_root(int x) const {
    return ch0_[fa_[x]] != x && ch1_[fa_[x]] != x;
  }
  void push_up(int x) {
    siz_[x] = siz_[ch0_[x]] + siz_[ch1_[x]] + 1;
    sum_[x] = sum_[ch0_[x]] + sum_[ch1_[x]] + val_[x];
  }
  void apply_add(int x, T v) {
    if (!x) return;
    val_[x] += v;
    sum_[x] += v * siz_[x];
    add_[x] += v;
  }
  void apply_rev(int x) {
    if (!x) return;
    std::swap(ch0_[x], ch1_[x]);
    rev_[x] ^= 1;
  }
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
  // x 绕父节点 y 旋转
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

 public:
  explicit LinkCutTree(int n) : fa_(n + 1, 0), ch0_(n + 1, 0), ch1_(n + 1, 0),
                               rev_(n + 1, 0), siz_(n + 1, 0),
                               val_(n + 1, T()), sum_(n + 1, T()), add_(n + 1, T()) {
    for (int i = 1; i <= n; ++i) siz_[i] = 1;
  }

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

  /// 打通 x 到根路径为实链
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

  void makeroot(int x) {
    access(x);
    apply_rev(x);
  }

  int findroot(int x) {
    access(x);
    while (ch0_[x]) {
      push_down(x);
      x = ch0_[x];
    }
    splay(x);
    return x;
  }

  void link(int u, int v) {
    makeroot(u);
    if (findroot(v) != u) fa_[u] = v;
  }

  void cut(int u, int v) {
    makeroot(u);
    access(v);
    if (ch0_[v] == u && ch1_[u] == 0) {
      ch0_[v] = 0;
      fa_[u] = 0;
      push_up(v);
    }
  }

  void split(int u, int v) {
    makeroot(u);
    access(v);
    // 此时 splay 根为 v，u 为 v 左子树最左端
  }

  T path_query(int u, int v) {
    split(u, v);
    return sum_[v];
  }

  void path_add(int u, int v, T delta) {
    split(u, v);
    apply_add(v, delta);
  }

  void set(int x, T v) {
    access(x);
    val_[x] = v;
    push_up(x);
  }
};

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_LINK_CUT_TREE_HPP