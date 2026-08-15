#ifndef WBWLIB_GRAPH_LCA_HPP
#define WBWLIB_GRAPH_LCA_HPP

/**
 * @file lca.hpp
 * @brief LCA：倍增法（O(n log n) 预处理，O(log n) 查询）+ 树深/距离/第 k 祖先。
 *
 * 依赖：wbwlib/core/base.hpp、wbwlib/graph/adjacency.hpp
 *
 * 用法：
 *   wbwlib::graph::LCA lc(g, 1);        // g 为树的邻接表（无向）
 *   int w = lc.query(u, v);
 *   int k = lc.kth(u, k);               // u 向上第 k 个祖先（k>=1）
 *   i64 d = lc.dist(u, v);              // 需要边权时用 WeightedLCA
 */

#include <vector>
#include <functional>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/graph/adjacency.hpp"

namespace wbwlib {
namespace graph {

class LCA {
 public:
  std::vector<int> fa, dep, sz;
  std::vector<int> tin, tout;     ///< DFS 序
  int LOG;

  explicit LCA(const Adj& g, int root = 1) { build(g, root); }

  void build(const Adj& g, int root = 1) {
    int n = (int)g.size() - 1;
    LOG = 1;
    while ((1 << LOG) <= n) ++LOG;
    up_.assign(n + 1, std::vector<int>(LOG, 0));
    fa.assign(n + 1, 0);
    dep.assign(n + 1, 0);
    sz.assign(n + 1, 0);
    tin.assign(n + 1, 0);
    tout.assign(n + 1, 0);
    int timer = 0;
    std::function<void(int, int)> dfs = [&](int u, int p) {
      fa[u] = p;
      dep[u] = dep[p] + 1;
      sz[u] = 1;
      tin[u] = ++timer;
      for (int v : g[u]) {
        if (v == p) continue;
        dfs(v, u);
        sz[u] += sz[v];
      }
      tout[u] = ++timer;
    };
    dfs(root, 0);
    for (int u = 1; u <= n; ++u) up_[u][0] = fa[u];
    for (int j = 1; j < LOG; ++j)
      for (int u = 1; u <= n; ++u)
        up_[u][j] = up_[up_[u][j - 1]][j - 1];
  }

  /// u 是否是 v 的祖先（含自身）
  bool is_ancestor(int u, int v) const {
    return tin[u] <= tin[v] && tout[v] <= tout[u];
  }

  int query(int u, int v) const {
    if (is_ancestor(u, v)) return u;
    if (is_ancestor(v, u)) return v;
    for (int j = LOG - 1; j >= 0; --j)
      if (up_[u][j] && !is_ancestor(up_[u][j], v)) u = up_[u][j];
    return up_[u][0];
  }

  /// u 向上第 k 个祖先（k>=1；超出返回 0）
  int kth(int u, int k) const {
    for (int j = 0; k; ++j, k >>= 1)
      if (k & 1) {
        u = up_[u][j];
        if (u == 0) break;
      }
    return u;
  }

 private:
  std::vector<std::vector<int>> up_;
};

/// 带边权 LCA：额外维护到根的距离，支持树上任意两点距离
template<class W>
class WeightedLCA {
 public:
  std::vector<i64> dist_to_root;   ///< 点到根的距离（边权和）
  LCA lc;

  explicit WeightedLCA(const WAdj<W>& g, int root = 1) : lc(unweighted(g), root) {
    int n = g.n;
    dist_to_root.assign(n + 1, 0);
    std::function<void(int, int, i64)> dfs = [&](int u, int p, i64 d) {
      dist_to_root[u] = d;
      for (auto& e : g.g[u])
        if (e.first != p) dfs(e.first, u, d + e.second);
    };
    dfs(root, 0, 0);
  }

  i64 dist(int u, int v) const {
    int w = lc.query(u, v);
    return dist_to_root[u] + dist_to_root[v] - 2 * dist_to_root[w];
  }

  int get_lca(int u, int v) const { return lc.query(u, v); }

 private:
  static Adj unweighted(const WAdj<W>& g) {
    int n = g.n;
    Adj uw(n + 1);
    for (int u = 1; u <= n; ++u)
      for (auto& e : g.g[u]) uw[u].push_back(e.first);
    return uw;
  }
};

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_LCA_HPP