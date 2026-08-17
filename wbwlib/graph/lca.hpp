#ifndef WBWLIB_GRAPH_LCA_HPP
#define WBWLIB_GRAPH_LCA_HPP

/**
 * @file lca.hpp
 * @brief LCA：倍增法（O(n log n) 预处理，O(log n) 查询）+ 树深/距离/第 k 祖先。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/graph/adjacency.hpp
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::graph::LCA lc(g, 1);        // g 为树的邻接表（无向）
 *   int w = lc.query(u, v);
 *   int k = lc.kth(u, k);               // u 向上第 k 个祖先（k>=1）
 *   i64 d = lc.dist(u, v);              // 需要边权时用 WeightedLCA
 * @endcode
 */

#include <vector>
#include <functional>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/graph/adjacency.hpp"

namespace wbwlib {
namespace graph {

/**
 * @brief LCA：倍增法，O(n log n) 预处理、O(log n) 查询，附树深/子树大小/DFS 序/第 k 祖先。
 */
class LCA {
 public:
  std::vector<int> fa, dep, sz;
  std::vector<int> tin, tout;     ///< DFS 序（进出时间戳）
  int LOG;

  /**
   * @brief 由树邻接表构建 LCA 倍增表。
   * @param g 树的邻接表（无向，1 基）
   * @param root 根节点，默认 1
   */
  explicit LCA(const Adj& g, int root = 1) { build(g, root); }

  /**
   * @brief 构建：DFS 求 fa/dep/sz/tin/tout，再倍增填 up 表。
   * @param g 树的邻接表（无向，1 基）
   * @param root 根节点，默认 1
   */
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

  /**
   * @brief 判断 u 是否是 v 的祖先（含自身）。
   * @param u 候选祖先（1 基）
   * @param v 后代（1 基）
   * @return 是祖先返回 true
   */
  bool is_ancestor(int u, int v) const {
    return tin[u] <= tin[v] && tout[v] <= tout[u];
  }

  /**
   * @brief 求 u 与 v 的最近公共祖先。
   * @param u 点（1 基）
   * @param v 点（1 基）
   * @return LCA 节点编号
   */
  int query(int u, int v) const {
    if (is_ancestor(u, v)) return u;
    if (is_ancestor(v, u)) return v;
    for (int j = LOG - 1; j >= 0; --j)
      if (up_[u][j] && !is_ancestor(up_[u][j], v)) u = up_[u][j];
    return up_[u][0];
  }

  /**
   * @brief u 向上第 k 个祖先。
   * @param u 起始点（1 基）
   * @param k 向上步数（k >= 1）
   * @return 第 k 个祖先编号；超出树根返回 0
   */
  int kth(int u, int k) const {
    for (int j = 0; k; ++j, k >>= 1)
      if (k & 1) {
        u = up_[u][j];
        if (u == 0) break;
      }
    return u;
  }

 private:
  /// 倍增表：up[u][j] 为 u 向上 2^j 层的祖先
  std::vector<std::vector<int>> up_;
};

/**
 * @brief 带边权 LCA：额外维护各点到根的距离，支持 O(log n) 求树上任意两点距离。
 * @tparam W 边权类型
 */
template<class W>
class WeightedLCA {
 public:
  std::vector<i64> dist_to_root;   ///< 点到根的距离（边权和）
  LCA lc;

  /**
   * @brief 构建：由带权邻接表建无权 LCA，并 DFS 累计到根距离。
   * @param g 树的带权邻接表（无向，1 基）
   * @param root 根节点，默认 1
   */
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

  /**
   * @brief 求 u 与 v 的树上距离，\f$dist(u,v) = d[u] + d[v] - 2 \cdot d[lca(u,v)]\f$。
   * @param u 点（1 基）
   * @param v 点（1 基）
   * @return 距离（边权和）
   */
  i64 dist(int u, int v) const {
    int w = lc.query(u, v);
    return dist_to_root[u] + dist_to_root[v] - 2 * dist_to_root[w];
  }

  /**
   * @brief 求 u 与 v 的最近公共祖先。
   * @param u 点（1 基）
   * @param v 点（1 基）
   * @return LCA 节点编号
   */
  int get_lca(int u, int v) const { return lc.query(u, v); }

 private:
  /// 由带权邻接表剥离出无权邻接表（仅保留邻点）
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