#ifndef WBWLIB_GRAPH_TARJAN_HPP
#define WBWLIB_GRAPH_TARJAN_HPP

/**
 * @file tarjan.hpp
 * @brief 强连通分量 / 割点 / 桥（Tarjan）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/graph/adjacency.hpp
 *
 * @par 复杂度
 * O(V+E)。
 *
 * @par 示例
 * @code{.cpp}
 *   int scc_cnt = tarjan_scc(g, scc_id);                // scc_id[1..n]，从 1 编号
 *   vector<int> is_cut = tarjan_cut(g);                 // 0/1
 *   vector<pair<int,int>> bri; tarjan_bridge(g, bri);   // 桥（u<v）
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
 * @brief 求有向图强连通分量（非递归标记式 Tarjan）。
 * @param g 无权邻接表（1 基）
 * @param scc_id 输出参数，接收每个点所属 SCC 编号（1 基，从 1 编号）
 * @return SCC 总数
 */
inline int tarjan_scc(const Adj& g, std::vector<int>& scc_id) {
  int n = (int)g.size() - 1;
  scc_id.assign(n + 1, 0);
  std::vector<int> dfn(n + 1, 0), low(n + 1, 0), stk;
  std::vector<char> in_stk(n + 1, 0);
  int idx = 0, scc = 0;
  std::function<void(int)> dfs = [&](int u) {
    dfn[u] = low[u] = ++idx;
    stk.push_back(u);
    in_stk[u] = 1;
    for (int v : g[u]) {
      if (!dfn[v]) {
        dfs(v);
        low[u] = std::min(low[u], low[v]);
      } else if (in_stk[v]) {
        low[u] = std::min(low[u], dfn[v]);
      }
    }
    if (low[u] == dfn[u]) {
      ++scc;
      while (true) {
        int x = stk.back();
        stk.pop_back();
        in_stk[x] = 0;
        scc_id[x] = scc;
        if (x == u) break;
      }
    }
  };
  for (int u = 1; u <= n; ++u)
    if (!dfn[u]) dfs(u);
  return scc;
}

/**
 * @brief 求无向图割点：\f$low[v] \ge dfn[u]\f$ 且 u 非根，或根有 ≥2 个子树。
 * @param g 无权邻接表（1 基）
 * @return cut[1..n]：1 表示该点为割点
 */
inline std::vector<int> tarjan_cut(const Adj& g) {
  int n = (int)g.size() - 1;
  std::vector<int> dfn(n + 1, 0), low(n + 1, 0), cut(n + 1, 0);
  int idx = 0;
  std::function<void(int, int)> dfs = [&](int u, int fa) {
    dfn[u] = low[u] = ++idx;
    int child = 0;
    for (int v : g[u]) {
      if (v == fa) continue;
      if (!dfn[v]) {
        ++child;
        dfs(v, u);
        low[u] = std::min(low[u], low[v]);
        if (fa != 0 && low[v] >= dfn[u]) cut[u] = 1;
      } else {
        low[u] = std::min(low[u], dfn[v]);
      }
    }
    if (fa == 0 && child >= 2) cut[u] = 1;
  };
  for (int u = 1; u <= n; ++u)
    if (!dfn[u]) dfs(u, 0);
  return cut;
}

/**
 * @brief 求无向图桥：\f$low[v] > dfn[u]\f$ 时边 (u,v) 为桥。
 * @param g 无权邻接表（1 基）
 * @param bridges 输出参数，接收所有桥（端点 u<v 有序）
 */
inline void tarjan_bridge(const Adj& g, std::vector<std::pair<int, int>>& bridges) {
  int n = (int)g.size() - 1;
  std::vector<int> dfn(n + 1, 0), low(n + 1, 0);
  int idx = 0;
  bridges.clear();
  // 给每条无向边一个 id，建立「点 → (邻点, 边id)」邻接，正确跳过反向重边
  std::vector<std::vector<std::pair<int, int>>> adj(n + 1);
  std::vector<int> eu, ev;
  for (int u = 1; u <= n; ++u)
    for (int v : g[u]) {
      if (u >= v) continue;
      int id = (int)eu.size() + 1;
      eu.push_back(u);
      ev.push_back(v);
      adj[u].push_back({v, id});
      adj[v].push_back({u, id});
    }
  std::function<void(int, int)> dfs = [&](int u, int pe) {
    dfn[u] = low[u] = ++idx;
    for (auto& e : adj[u]) {
      int v = e.first, id = e.second;
      if (id == pe) continue;
      if (!dfn[v]) {
        dfs(v, id);
        low[u] = std::min(low[u], low[v]);
        if (low[v] > dfn[u]) bridges.push_back({std::min(u, v), std::max(u, v)});
      } else {
        low[u] = std::min(low[u], dfn[v]);
      }
    }
  };
  for (int u = 1; u <= n; ++u)
    if (!dfn[u]) dfs(u, 0);
}

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_TARJAN_HPP