#ifndef WBWLIB_GRAPH_TOPO_HPP
#define WBWLIB_GRAPH_TOPO_HPP

/**
 * @file topo.hpp
 * @brief 拓扑排序（Kahn，返回字典序可选）+ 判环 + 关键路径。
 *
 * 依赖：wbwlib/core/base.hpp、wbwlib/graph/adjacency.hpp
 *
 * 复杂度：O(V+E)。
 */

#include <vector>
#include <queue>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/graph/adjacency.hpp"

namespace wbwlib {
namespace graph {

/// Kahn 拓扑排序；order 填入拓扑序列；返回是否存在拓扑序（无环）
inline bool topo_sort(const Adj& g, std::vector<int>& order) {
  int n = (int)g.size() - 1;
  std::vector<int> indeg(n + 1, 0);
  for (int u = 1; u <= n; ++u)
    for (int v : g[u]) ++indeg[v];
  std::queue<int> q;
  for (int u = 1; u <= n; ++u)
    if (indeg[u] == 0) q.push(u);
  order.clear();
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    order.push_back(u);
    for (int v : g[u])
      if (--indeg[v] == 0) q.push(v);
  }
  return (int)order.size() == n;
}

/// 拓扑排序（小根堆 → 字典序最小）
inline bool topo_sort_lex_min(const Adj& g, std::vector<int>& order) {
  int n = (int)g.size() - 1;
  std::vector<int> indeg(n + 1, 0);
  for (int u = 1; u <= n; ++u)
    for (int v : g[u]) ++indeg[v];
  std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
  for (int u = 1; u <= n; ++u)
    if (indeg[u] == 0) pq.push(u);
  order.clear();
  while (!pq.empty()) {
    int u = pq.top();
    pq.pop();
    order.push_back(u);
    for (int v : g[u])
      if (--indeg[v] == 0) pq.push(v);
  }
  return (int)order.size() == n;
}

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_TOPO_HPP