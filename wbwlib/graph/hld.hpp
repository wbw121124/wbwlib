#ifndef WBWLIB_GRAPH_HLD_HPP
#define WBWLIB_GRAPH_HLD_HPP

/**
 * @file hld.hpp
 * @brief 树链剖分（HLD）：剖分 + 路径/子树区间映射，供线段树等配合。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/graph/adjacency.hpp
 *
 * @par 复杂度
 * 预处理 O(n)，单次路径 O(log^2 n) 段，子树 O(1) 区间。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::graph::HLD hld(g, 1);
 *   hld.path(u, v, [&](int l, int r){ seg.update(l, r, x); });  // 含重链区间，顺序为 u→v
 *   hld.subtree(u, [&](int l, int r){ ... });                    // 子树对应区间 [l,r]
 *   节点权值直接用 posIn[u]（轻链底端朝上）；边权把权挂到子节点。
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
 * @brief 树链剖分 HLD：轻重链剖分，把路径/子树映射为基础数组区间，供线段树等配合。
 */
class HLD {
 public:
  std::vector<int> fa, dep, sz, son, top, pos;  ///< pos = 点在基础数组中的下标（1 基）
  std::vector<int> order;                       ///< 剖分顺序（按重链）
  int n, timer;

  /**
   * @brief 构建树链剖分。
   * @param g 树的邻接表（无向，1 基）
   * @param root 根节点，默认 1
   */
  HLD(const Adj& g, int root = 1) { build(g, root); }

  /**
   * @brief 构建：第一遍 DFS 求 fa/dep/sz/son，第二遍重儿子优先线性化。
   * @param g 树的邻接表（无向，1 基）
   * @param root 根节点，默认 1
   */
  void build(const Adj& g, int root = 1) {
    n = (int)g.size() - 1;
    timer = 0;
    fa.assign(n + 1, 0);
    dep.assign(n + 1, 0);
    sz.assign(n + 1, 0);
    son.assign(n + 1, 0);
    top.assign(n + 1, 0);
    pos.assign(n + 1, 0);
    order.assign(n + 1, 0);
    std::vector<int> parent(n + 1, 0);
    // 第一遍：deep/sz/son
    std::function<void(int, int)> dfs1 = [&](int u, int p) {
      fa[u] = p;
      dep[u] = dep[p] + 1;
      sz[u] = 1;
      int best = 0;
      for (int v : g[u]) {
        if (v == p) continue;
        dfs1(v, u);
        sz[u] += sz[v];
        if (sz[v] > best) { best = sz[v]; son[u] = v; }
      }
      parent[u] = p;
    };
    dfs1(root, 0);
    // 第二遍：top + 重链线性化（重儿子优先），再轻儿子
    std::function<void(int, int)> dfs2 = [&](int u, int t) {
      top[u] = t;
      pos[u] = ++timer;
      order[timer] = u;
      if (son[u]) dfs2(son[u], t);
      for (int v : g[u])
        if (v != fa[u] && v != son[u]) dfs2(v, v);
    };
    dfs2(root, root);
  }

  /**
   * @brief 枚举 u 子树对应的区间，回调 cb(l, r)（连续区间 [pos[u], pos[u]+sz[u]-1]）。
   * @tparam F 回调类型
   * @param u 子树根（1 基）
   * @param cb 接收区间 [l, r] 的回调
   */
  template<class F>
  void subtree(int u, F&& cb) {
    cb(pos[u], pos[u] + sz[u] - 1);
  }

  /**
   * @brief 枚举 u→v 路径拆成的 [l,r] 段（重链顶端朝下递增），顺序为从 u 一直到 v。
   * @tparam F 回调类型
   * @param u 路径起点（1 基）
   * @param v 路径终点（1 基）
   * @param cb 按顺序接收每段区间 [l, r] 的回调
   */
  template<class F>
  void path(int u, int v, F&& cb) {
    std::vector<std::pair<int, int>> left, right;
    while (top[u] != top[v]) {
      if (dep[top[u]] >= dep[top[v]]) {
        left.push_back({pos[top[u]], pos[u]});
        u = fa[top[u]];
      } else {
        right.push_back({pos[top[v]], pos[v]});
        v = fa[top[v]];
      }
    }
    if (dep[u] >= dep[v]) left.push_back({pos[v], pos[u]});
    else right.push_back({pos[u], pos[v]});
    // 反转 right，得到 u→v 的正确顺序
    for (auto& p : left) cb(p.first, p.second);
    for (int i = (int)right.size() - 1; i >= 0; --i)
      cb(right[i].first, right[i].second);
  }

  /**
   * @brief 求 u 与 v 的最近公共祖先（利用链顶跳跃）。
   * @param u 点（1 基）
   * @param v 点（1 基）
   * @return LCA 节点编号
   */
  int lca(int u, int v) const {
    while (top[u] != top[v]) {
      if (dep[top[u]] < dep[top[v]]) std::swap(u, v);
      u = fa[top[u]];
    }
    return dep[u] < dep[v] ? u : v;
  }
};

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_HLD_HPP