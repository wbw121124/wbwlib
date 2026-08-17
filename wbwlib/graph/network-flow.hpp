#ifndef WBWLIB_GRAPH_NETWORK_FLOW_HPP
#define WBWLIB_GRAPH_NETWORK_FLOW_HPP

/**
 * @file network-flow.hpp
 * @brief 最大流：Dinic（当前弧优化 + 分层）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * 一般图 O(E V^2)；单位容量 O(E sqrt V)。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::graph::Dinic<i64> din(n);
 *   din.add_edge(u, v, cap);
 *   i64 f = din.maxflow(s, t);
 *   din.cut(s, t, st);    // 最小割点集
 * @endcode
 */

#include <vector>
#include <queue>
#include <limits>
#include <functional>
#include <algorithm>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace graph {

/**
 * @brief 最大流：Dinic 算法（BFS 分层 + DFS 多路增广 + 当前弧优化）。
 * @tparam F 流量类型（int / i64 等）
 */
template<class F>
class Dinic {
  struct Edge { int to, rev; F cap; };
  std::vector<std::vector<Edge>> g_;
  std::vector<int> level_, iter_;
  int n_;

  /// 分层 BFS：只走剩余容量 > 0 的边，构造层次图，判断 s 是否可达 t
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

  /// 沿层次图 DFS 单路增广：只走 level[v] = level[u] + 1 的边，并更新反向边容量
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

  /// 从 s 沿剩余容量 > 0 的边 BFS，标记残量网络中 s 可达的点（最小割 S 侧）
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

  /**
   * @brief 构造 n 个点的空网络。
   * @param n 点数（1 基）
   */
 public:
  explicit Dinic(int n) : g_(n + 1), n_(n) {}

  /**
   * @brief 加一条 u → v 容量为 cap 的有向边（自动加容量 0 的反向边）。
   * @param u 起点（1 基）
   * @param v 终点（1 基）
   * @param cap 容量
   */
  void add_edge(int u, int v, F cap) {
    g_[u].push_back({v, (int)g_[v].size(), cap});
    g_[v].push_back({u, (int)g_[u].size() - 1, 0});
  }

  /**
   * @brief 求 s → t 的最大流：不断「分层 + 多路增广」直到 s 不可达 t，\f$|f| = \sum_{e \in \delta^+(s)} f(e)\f$。
   * @param s 源点（1 基）
   * @param t 汇点（1 基）
   * @return 最大流值
   */
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

  /**
   * @brief 最小割：最大流结束后返回残量网络中 s 可达的点集（S 侧，含 s）。
   * @param s 源点（1 基）
   * @param t 汇点（1 基）
   * @return S 侧点集标记数组（1 基）
   */
  std::vector<char> mincut(int s, int t) {
    (void)t;
    std::vector<char> vis;
    build_cut(s, vis);
    return vis;
  }

 private:
  /// 返回类型 F 的安全上界（numeric_limits 最大值 / 4）
  static F max_val() { return std::numeric_limits<F>::max() / 4; }
};

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_NETWORK_FLOW_HPP