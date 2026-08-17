#ifndef WBWLIB_GRAPH_SHORTEST_PATH_HPP
#define WBWLIB_GRAPH_SHORTEST_PATH_HPP

/**
 * @file shortest-path.hpp
 * @brief 最短路：Dijkstra（堆）/ SPFA（判负环）/ Floyd / 0-1 BFS / 拓扑序 DAG。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/graph/adjacency.hpp
 *
 * 复杂度：
 *   dijkstra     O((V+E) log V)，要求边权非负。
 *   spfa         O(VE) 最坏，可判负环；返回 false 表示存在负环。
 *   floyd        O(V^3)，任意两点最短路（INF= 不存在）。
 *   bfs01        O(V+E)，边权 ∈ {0,1}。
 *   dag_shortest O(V+E)，DAG 最短路/最长路（先拓扑序再 DP）。
 */

#include <vector>
#include <queue>
#include <deque>
#include <limits>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/graph/adjacency.hpp"

namespace wbwlib {
namespace graph {

/// 表示无穷大（不可达 / 初始距离）
constexpr i64 INF_LL = (i64)4e18;

/**
 * @brief Dijkstra 单源最短路（要求边权非负）：每次取堆顶未确定点做松弛。
 * @tparam W 边权类型
 * @param n 点数（1 基）
 * @param s 源点
 * @param g 带权邻接表
 * @return dist[1..n]，不可达为 INF_LL
 */
template<class W>
std::vector<i64> dijkstra(int n, int s, const WAdj<W>& g) {
  std::vector<i64> dist(n + 1, INF_LL);
  typedef std::pair<i64, int> P;
  std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
  dist[s] = 0;
  pq.push({0, s});
  while (!pq.empty()) {
    int u = pq.top().second;
    i64 d = pq.top().first;
    pq.pop();
    if (d != dist[u]) continue;
    for (auto& e : g.g[u]) {
      int v = e.first;
      if (dist[u] + e.second < dist[v]) {
        dist[v] = dist[u] + e.second;
        pq.push({dist[v], v});
      }
    }
  }
  return dist;
}

/**
 * @brief SPFA 单源最短路（可判负环）：入队次数 ≥ n 即判定存在负环。
 * @tparam W 边权类型
 * @param n 点数（1 基）
 * @param s 源点
 * @param g 带权邻接表
 * @param dist 输出参数，接收 dist[1..n]（检测到负环时部分结果可能无效）
 * @return 无负环返回 true；存在负环返回 false
 */
template<class W>
bool spfa(int n, int s, const WAdj<W>& g, std::vector<i64>& dist) {
  dist.assign(n + 1, INF_LL);
  std::vector<int> cnt(n + 1, 0);
  std::vector<char> inq(n + 1, 0);
  std::queue<int> q;
  dist[s] = 0;
  inq[s] = 1;
  q.push(s);
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    inq[u] = 0;
    for (auto& e : g.g[u]) {
      int v = e.first;
      if (dist[u] + e.second < dist[v]) {
        dist[v] = dist[u] + e.second;
        if (!inq[v]) {
          if (++cnt[v] >= n) return false;   // 入队次数 ≥ n → 负环
          inq[v] = 1;
          q.push(v);
        }
      }
    }
  }
  return true;
}

/**
 * @brief Floyd 任意两点最短路：递推 \f$d[i][j] = \min(d[i][j],\; d[i][k]+d[k][j])\f$。
 * @param d n×n 邻接矩阵（下标 1 基；i==j 置 0，无边置 INF_LL），结果就地写入
 */
inline void floyd(std::vector<std::vector<i64>>& d) {
  int n = (int)d.size() - 1;
  for (int k = 1; k <= n; ++k)
    for (int i = 1; i <= n; ++i) {
      if (d[i][k] >= INF_LL) continue;
      for (int j = 1; j <= n; ++j)
        if (d[k][j] < INF_LL && d[i][k] + d[k][j] < d[i][j])
          d[i][j] = d[i][k] + d[k][j];
    }
}

/**
 * @brief 0-1 BFS：边权仅 0/1 时按权值插入队首/队尾求单源最短路。
 * @tparam W 边权类型（须为 0 或 1）
 * @param n 点数（1 基）
 * @param s 源点
 * @param g 带权邻接表
 * @return dist[1..n]，不可达为 INF_LL
 */
template<class W>
std::vector<i64> bfs01(int n, int s, const WAdj<W>& g) {
  std::deque<int> dq;
  std::vector<i64> dist(n + 1, INF_LL);
  dist[s] = 0;
  dq.push_front(s);
  while (!dq.empty()) {
    int u = dq.front();
    dq.pop_front();
    for (auto& e : g.g[u]) {
      i64 nd = dist[u] + e.second;
      if (nd < dist[e.first]) {
        dist[e.first] = nd;
        if (e.second == 0) dq.push_front(e.first);
        else dq.push_back(e.first);
      }
    }
  }
  return dist;
}

/**
 * @brief DAG 最短路：先 Kahn 拓扑排序，再按拓扑序从点 1 出发做松弛 DP（不支持负环语义，DAG 无环）。
 * @tparam W 边权类型
 * @param n 点数（1 基）
 * @param g 带权邻接表
 * @param order 输出参数，接收拓扑序
 * @return dist[1..n]（从点 1 出发，不可达为 INF_LL）
 */
template<class W>
std::vector<i64> dag_shortest(int n, const WAdj<W>& g,
                              std::vector<int>& order) {
  std::vector<int> indeg(n + 1, 0);
  for (int u = 1; u <= n; ++u)
    for (auto& e : g.g[u]) ++indeg[e.first];
  std::queue<int> q;
  for (int u = 1; u <= n; ++u)
    if (indeg[u] == 0) q.push(u);
  order.clear();
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    order.push_back(u);
    for (auto& e : g.g[u])
      if (--indeg[e.first] == 0) q.push(e.first);
  }
  // 不支持负权处理；DP 求从 1 出发
  std::vector<i64> dist(n + 1, INF_LL);
  dist[1] = 0;
  for (int u : order)
    if (dist[u] < INF_LL)
      for (auto& e : g.g[u])
        if (dist[u] + e.second < dist[e.first])
          dist[e.first] = dist[u] + e.second;
  return dist;
}

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_SHORTEST_PATH_HPP