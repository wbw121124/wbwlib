#ifndef WBWLIB_GRAPH_MST_HPP
#define WBWLIB_GRAPH_MST_HPP

/**
 * @file mst.hpp
 * @brief 最小生成树：Kruskal（并查集）+ Prim（堆）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/graph/adjacency.hpp
 *
 * @par 复杂度
 * Kruskal O(E log E)，Prim O(E log V)。
 *
 * 返回 MstResult{ total, edges, ok }；不连通则 ok=false。
 */

#include <vector>
#include <queue>
#include <array>
#include <functional>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/graph/adjacency.hpp"

namespace wbwlib {
namespace graph {

/// MST 使用的无穷大上界
inline i64 mst_INF() { return (i64)4e18; }

/**
 * @brief 最小生成树结果：总权值 + 树边 + 是否连通。
 * @tparam W 权值类型
 */
template<class W>
struct MstResult {
  i64 total = 0;
  std::vector<std::pair<int, int>> edges;  ///< 树边
  bool ok = true;
};

/**
 * @brief 从带权邻接表中抽出无向边集 {u,v,w}（只取 u<v 的一份）。
 * @tparam W 边权类型
 * @param g 带权邻接表（1 基）
 * @return 边数组，每条为 {u, v, w}（1 基）
 */
template<class W>
inline std::vector<std::array<int, 3>> extract_edges(const WAdj<W>& g) {
  std::vector<std::array<int, 3>> e;
  for (int u = 1; u <= g.n; ++u)
    for (auto& p : g.g[u])
      if (u < p.first) e.push_back({u, p.first, (int)p.second});
  return e;
}

/**
 * @brief Kruskal 求最小生成树：边按权值升序，并查集判连通合并。
 * @param n 点数（1 基）
 * @param edges 边集，每条为 {u, v, w}（1 基）
 * @return MstResult；图不连通时 ok = false
 */
inline MstResult<i64> kruskal(int n,
                              const std::vector<std::array<int, 3>>& edges) {
  MstResult<i64> res;
  std::vector<int> fa(n + 1), rnk(n + 1, 0);
  for (int i = 1; i <= n; ++i) fa[i] = i;
  std::function<int(int)> find = [&](int x) {
    while (fa[x] != x) x = fa[x] = fa[fa[x]];
    return x;
  };
  auto es = edges;
  std::sort(es.begin(), es.end(),
            [](const std::array<int, 3>& a, const std::array<int, 3>& b) {
              return a[2] < b[2];
            });
  int cnt = 0;
  for (auto& e : es) {
    int u = find(e[0]), v = find(e[1]);
    if (u == v) continue;
    res.edges.push_back({e[0], e[1]});
    res.total += e[2];
    if (rnk[u] < rnk[v]) std::swap(u, v);
    fa[v] = u;
    if (rnk[u] == rnk[v]) ++rnk[u];
    if (++cnt == n - 1) break;
  }
  res.ok = (cnt == n - 1);
  return res;
}

/**
 * @brief 从带权邻接表直接跑 Kruskal。
 * @param g 带权邻接表（1 基）
 * @return MstResult；图不连通时 ok = false
 */
inline MstResult<i64> kruskal(const WAdj<i64>& g) {
  return kruskal(g.n, extract_edges(g));
}

/**
 * @brief Prim（堆）求最小生成树总权值：从点 1 出发不断加入最小 key 的未选点。
 * @tparam W 边权类型
 * @param n 点数（1 基）
 * @param g 带权邻接表
 * @return 最小生成树总权值；图不连通返回 -1
 */
template<class W>
i64 prim(int n, const WAdj<W>& g) {
  const i64 INF = mst_INF();
  std::vector<i64> key(n + 1, INF);
  std::vector<char> used(n + 1, 0);
  typedef std::pair<i64, int> P;
  std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
  key[1] = 0;
  pq.push({0, 1});
  i64 total = 0;
  int cnt = 0;
  while (!pq.empty()) {
    int u = pq.top().second;
    i64 d = pq.top().first;
    pq.pop();
    if (used[u]) continue;
    used[u] = 1;
    total += d;
    ++cnt;
    for (auto& e : g.g[u])
      if (!used[e.first] && e.second < key[e.first]) {
        key[e.first] = e.second;
        pq.push({key[e.first], e.first});
      }
  }
  return cnt == n ? total : -1;
}

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_MST_HPP