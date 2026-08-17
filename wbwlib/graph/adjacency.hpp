#ifndef WBWLIB_GRAPH_ADJACENCY_HPP
#define WBWLIB_GRAPH_ADJACENCY_HPP

/**
 * @file adjacency.hpp
 * @brief 图表示约定：1 基邻接表 + 常用辅助。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * 约定：
 *   - 所有图算法使用 1..n 的点编号。
 *   - 无权图：Adj = vector<vector<int>>（大小为 n+1）。
 *   - 带权图：WAdj<W> 结构体（成员 g[n+1]，pair(v,w)）。
 *   - 树：同上，通常建双向边。
 */

#include <vector>
#include <utility>
#include <tuple>
#include <algorithm>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace graph {

/// 无权邻接表（1 基）
using Adj = std::vector<std::vector<int>>;

/**
 * @brief 带权邻接表（1 基）：g[u] 为 (邻点 v, 边权 w) 列表。
 * @tparam W 边权类型，默认 i64
 */
template<class W = i64>
struct WAdj {
  int n;
  std::vector<std::vector<std::pair<int, W>>> g;
  /**
   * @brief 构造 n 个点的带权邻接表。
   * @param n_ 点数
   */
  explicit WAdj(int n_ = 0) : n(n_), g(n_ + 1) {}
  /**
   * @brief 加一条 u → v 的有向边。
   * @param u 起点（1 基）
   * @param v 终点（1 基）
   * @param w 边权
   */
  void add(int u, int v, W w) { g[u].push_back({v, w}); }
  /**
   * @brief 加一条 u–v 无向边（双向）。
   * @param u 端点（1 基）
   * @param v 端点（1 基）
   * @param w 边权
   */
  void add_bidir(int u, int v, W w) { add(u, v, w); add(v, u, w); }
  /// 返回点数 n
  int size() const { return n; }
};

/**
 * @brief 由边列表建无权图。
 * @param n 点数
 * @param edges 边列表，元素为 (u, v)
 * @param bidir 为 true 时建无向图，否则只建有向边
 * @return 1 基无权邻接表
 */
inline Adj make_graph(int n, const std::vector<std::pair<int, int>>& edges, bool bidir = true) {
  Adj a(n + 1);
  for (auto& e : edges) {
    a[e.first].push_back(e.second);
    if (bidir) a[e.second].push_back(e.first);
  }
  return a;
}

/**
 * @brief 由边列表建带权图。
 * @tparam W 边权类型
 * @param n 点数
 * @param edges 边列表，元素为 (u, v, w)
 * @param bidir 为 true 时建无向图，否则只建有向边
 * @return 1 基带权邻接表
 */
template<class W>
WAdj<W> make_graph(int n, const std::vector<std::tuple<int, int, W>>& edges, bool bidir = true) {
  WAdj<W> a(n);
  for (auto& e : edges) {
    int u = std::get<0>(e), v = std::get<1>(e); W w = std::get<2>(e);
    a.add(u, v, w);
    if (bidir) a.add(v, u, w);
  }
  return a;
}

/**
 * @brief 向无权图加一条边（不检查重边）。
 * @param a 邻接表（1 基）
 * @param u 端点（1 基）
 * @param v 端点（1 基）
 * @param bidir 为 true 时加无向边，否则只加 u → v
 */
inline void add_edge(Adj& a, int u, int v, bool bidir = true) {
  a[u].push_back(v);
  if (bidir) a[v].push_back(u);
}

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_ADJACENCY_HPP