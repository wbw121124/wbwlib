#ifndef WBWLIB_GRAPH_NETWORK_FLOW_HPP
#define WBWLIB_GRAPH_NETWORK_FLOW_HPP

/**
 * @file network-flow.hpp
 * @brief 最大流：Dinic（当前弧优化 + 分层）。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：一般图 O(E V^2)；单位容量 O(E sqrt V)。
 *
 * 用法：
 *   wbwlib::graph::Dinic<i64> din(n);
 *   din.add_edge(u, v, cap);
 *   i64 f = din.maxflow(s, t);
 *   din.cut(s, t, st);    // 最小割点集
 */

#include <vector>
#include <queue>
#include <limits>
#include <functional>
#include <algorithm>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace graph {

template<class F>
class Dinic {
  struct Edge { int to, rev; F cap; };
  std::vector<std::vector<Edge>> g_;
  std::vector<int> level_, iter_;
  int n_;

  bool bfs(int s, int t) {
    level_.assign(n_ + 1, -1);
    std::queue<int> q;
    level_[s] = 0;
    q.push(s);
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (auto& e : g_[u])
        if (e.cap > 0 && level_[e.to] < 0) {
          level_[e.to] = level_[u] + 1;
          q.push(e.to);
        }
    }
    return level_[t] >= 0;
  }

  F dfs(int u, int t, F f) {
    if (u == t) return f;
    for (int& i = iter_[u]; i < (int)g_[u].size(); ++i) {
      Edge& e = g_[u][i];
      if (e.cap > 0 && level_[e.to] == level_[u] + 1) {
        F d = dfs(e.to, t, std::min(f, e.cap));
        if (d > 0) {
          e.cap -= d;
          g_[e.to][e.rev].cap += d;
          return d;
        }
      }
    }
    return 0;
  }

  void build_cut(int s, std::vector<char>& vis) {
    vis.assign(n_ + 1, 0);
    std::queue<int> q;
    vis[s] = 1;
    q.push(s);
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (auto& e : g_[u])
        if (!vis[e.to] && e.cap > 0) {
          vis[e.to] = 1;
          q.push(e.to);
        }
    }
  }

 public:
  explicit Dinic(int n) : g_(n + 1), n_(n) {}

  void add_edge(int u, int v, F cap) {
    g_[u].push_back({v, (int)g_[v].size(), cap});
    g_[v].push_back({u, (int)g_[u].size() - 1, 0});
  }

  F maxflow(int s, int t) {
    F flow = 0;
    const F INF = max_val();
    while (bfs(s, t)) {
      iter_.assign(n_ + 1, 0);
      F f;
      while ((f = dfs(s, t, INF)) > 0) flow += f;
    }
    return flow;
  }

  /// 最小割：返回 S 侧点集（含 s）
  std::vector<char> mincut(int s, int t) {
    (void)t;
    std::vector<char> vis;
    build_cut(s, vis);
    return vis;
  }

 private:
  static F max_val() { return std::numeric_limits<F>::max() / 4; }
};

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_NETWORK_FLOW_HPP