#ifndef WBWLIB_GRAPH_ADJACENCY_HPP
#define WBWLIB_GRAPH_ADJACENCY_HPP

/**
 * @file adjacency.hpp
 * @brief 图表示约定：1 基邻接表 + 常用辅助。
 *
 * 依赖：wbwlib/core/base.hpp
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

/// 带权邻接表（1 基）
template<class W = i64>
struct WAdj {
  int n;
  std::vector<std::vector<std::pair<int, W>>> g;
  explicit WAdj(int n_ = 0) : n(n_), g(n_ + 1) {}
  void add(int u, int v, W w) { g[u].push_back({v, w}); }
  void add_bidir(int u, int v, W w) { add(u, v, w); add(v, u, w); }
  int size() const { return n; }
};

/// 由边列表建无权图（edges 元素即 (u,v)）
inline Adj make_graph(int n, const std::vector<std::pair<int, int>>& edges, bool bidir = true) {
  Adj a(n + 1);
  for (auto& e : edges) {
    a[e.first].push_back(e.second);
    if (bidir) a[e.second].push_back(e.first);
  }
  return a;
}

/// 由边列表建带权图（edges 元素即 (u,v,w)）
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

/// 建一条无向边（不检查重边）
inline void add_edge(Adj& a, int u, int v, bool bidir = true) {
  a[u].push_back(v);
  if (bidir) a[v].push_back(u);
}

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_ADJACENCY_HPP