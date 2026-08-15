#ifndef WBWLIB_GRAPH_EULER_PATH_HPP
#define WBWLIB_GRAPH_EULER_PATH_HPP

/**
 * @file euler-path.hpp
 * @brief 欧拉路径 / 欧拉回路（Hierholzer，有向/无向）。
 *
 * 依赖：wbwlib/core/base.hpp、wbwlib/graph/adjacency.hpp
 *
 * 复杂度：O(V+E)。
 *
 * 用法（无向图，图需本身连通）：
 *   std::vector<int> path;
 *   bool ok = euler_undirected(g, path);   // path 为顶点序列（若存在）
 * 有向图：
 *   bool ok = euler_directed(g, path);
 *
 * 说明：实现为删边式 Hierholzer（边用 multiset 模拟删除）。
 */

#include <vector>
#include <set>
#include <functional>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/graph/adjacency.hpp"

namespace wbwlib {
namespace graph {

/// 无向图欧拉路径/回路。存在性由连通性 + 奇度点个数（≤2）决定。
/// path 返回顶点行走序列（含首尾）。若无欧拉路径返回 false。
inline bool euler_undirected(const Adj& g, std::vector<int>& path) {
  int n = (int)g.size() - 1;
  std::vector<int> deg(n + 1, 0);
  for (int u = 1; u <= n; ++u) deg[u] = (int)g[u].size();
  int odd = 0, start = 1;
  for (int u = 1; u <= n; ++u) {
    if (deg[u] == 0) continue;
    start = u;
    if (deg[u] & 1) { ++odd; start = u; }
  }
  if (odd != 0 && odd != 2) return false;
  // 边多重集（模拟删除；重边用 pair(u,v) 可删一份）
  int E = 0;
  std::vector<std::multiset<int>> ms(n + 1);
  for (int u = 1; u <= n; ++u)
    for (int v : g[u])
      if (u < v) { ms[u].insert(v); ms[v].insert(u); ++E; }
  path.clear();
  // Hierholzer 递归
  std::function<void(int)> dfs = [&](int u) {
    while (!ms[u].empty()) {
      auto it = ms[u].begin();
      int v = *it;
      ms[u].erase(it);
      auto it2 = ms[v].find(u);
      ms[v].erase(it2);
      dfs(v);
    }
    path.push_back(u);
  };
  dfs(start);
  if ((int)path.size() != E + 1) return false;
  std::reverse(path.begin(), path.end());
  return true;
}

/// 有向图欧拉路径/回路。要求底图弱连通。
inline bool euler_directed(const Adj& g, std::vector<int>& path) {
  int n = (int)g.size() - 1;
  std::vector<int> indeg(n + 1, 0), outdeg(n + 1, 0);
  for (int u = 1; u <= n; ++u) {
    outdeg[u] = (int)g[u].size();
    for (int v : g[u]) ++indeg[v];
  }
  int start = -1, end = -1;
  for (int u = 1; u <= n; ++u) {
    int d = outdeg[u] - indeg[u];
    if (d == 1) { if (start != -1) return false; start = u; }
    else if (d == -1) { if (end != -1) return false; end = u; }
    else if (d != 0) return false;
  }
  // 选起点
  std::vector<int> tmp;
  for (int u = 1; u <= n; ++u) if (outdeg[u] > 0 || indeg[u] > 0) tmp.push_back(u);
  if (start == -1) start = tmp.empty() ? 1 : tmp[0];
  // 删边式 DFS：复制出边
  std::vector<std::multiset<int>> ms(n + 1);
  for (int u = 1; u <= n; ++u)
    for (int v : g[u]) ms[u].insert(v);
  path.clear();
  std::function<void(int)> dfs = [&](int u) {
    while (!ms[u].empty()) {
      auto it = ms[u].begin();
      int v = *it;
      ms[u].erase(it);
      dfs(v);
    }
    path.push_back(u);
  };
  dfs(start);
  int total = 0;
  for (int u = 1; u <= n; ++u) total += outdeg[u];
  if ((int)path.size() != total + 1) return false;
  std::reverse(path.begin(), path.end());
  return true;
}

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_EULER_PATH_HPP