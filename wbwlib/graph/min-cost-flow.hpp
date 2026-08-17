#ifndef WBWLIB_GRAPH_MIN_COST_FLOW_HPP
#define WBWLIB_GRAPH_MIN_COST_FLOW_HPP

/**
 * @file min-cost-flow.hpp
 * @brief 最小费用最大流：SPFA 增广（允许负费用边，判负环场景另处理）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * SPFA 版 O(F·VE)（F 为流量）：负权单源最短路逐步增广。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::graph::MinCostFlow mcf(n);
 *   mcf.add_edge(u, v, cap, cost);
 *   auto [flow, cost] = mcf.min_cost_flow(s, t);   // C++17；C++14 用 .first/.second
 *   mcf.min_cost_flow(s, t, flow_limit);
 * @endcode
 */

#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace graph {

/**
 * @brief 最小费用最大流：SPFA 逐步找最短路增广（允许负费用边）。
 */
class MinCostFlow {
  struct Edge { int to, rev; i64 cap, cost; };
  std::vector<std::vector<Edge>> g_;
  int n_;

 public:
  /**
   * @brief 构造 n 个点的网络。
   * @param n 点数（1 基）
   */
  explicit MinCostFlow(int n) : g_(n + 1), n_(n) {}

  /**
   * @brief 加一条 u → v 容量 cap、单位费用 cost 的边（自动加容量 0、费用 -cost 的反向边）。
   * @param u 起点（1 基）
   * @param v 终点（1 基）
   * @param cap 容量
   * @param cost 单位费用
   */
  void add_edge(int u, int v, i64 cap, i64 cost) {
    g_[u].push_back({v, (int)g_[v].size(), cap, cost});
    g_[v].push_back({u, (int)g_[u].size() - 1, 0, -cost});
  }

  /**
   * @brief 求 s → t 的最小费用流：每轮 SPFA 求最短路并沿前驱增广，总费用 \f$\sum add \cdot dist[t]\f$。
   * @param s 源点（1 基）
   * @param t 汇点（1 基）
   * @param limit 流量上限；limit < 0 表示求最大流
   * @return pair{总流量, 总费用}
   */
  std::pair<i64, i64> min_cost_flow(int s, int t, i64 limit = -1) {
    const i64 INF = (i64)4e18;
    i64 flow = 0, cost = 0;
    while (limit < 0 || flow < limit) {
      std::vector<i64> dist(n_ + 1, INF);
      std::vector<char> inq(n_ + 1, 0);
      std::vector<int> pre(n_ + 1, -1), preE(n_ + 1, -1);
      std::queue<int> q;
      dist[s] = 0;
      inq[s] = 1;
      q.push(s);
      while (!q.empty()) {
        int u = q.front();
        q.pop();
        inq[u] = 0;
        for (int i = 0; i < (int)g_[u].size(); ++i) {
          const Edge& e = g_[u][i];
          if (e.cap > 0 && dist[u] + e.cost < dist[e.to]) {
            dist[e.to] = dist[u] + e.cost;
            pre[e.to] = u;
            preE[e.to] = i;
            if (!inq[e.to]) { inq[e.to] = 1; q.push(e.to); }
          }
        }
      }
      if (dist[t] >= INF) break;
      i64 add = limit < 0 ? INF : limit - flow;
      for (int v = t; v != s; v = pre[v]) add = std::min(add, g_[pre[v]][preE[v]].cap);
      for (int v = t; v != s; v = pre[v]) {
        Edge& e = g_[pre[v]][preE[v]];
        e.cap -= add;
        g_[v][e.rev].cap += add;
      }
      flow += add;
      cost += add * dist[t];
    }
    return {flow, cost};
  }
};

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_MIN_COST_FLOW_HPP