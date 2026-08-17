#ifndef WBWLIB_GRAPH_BIPARTITE_HPP
#define WBWLIB_GRAPH_BIPARTITE_HPP

/**
 * @file bipartite.hpp
 * @brief 二分图判定（黑白染色）+ 最大匹配（匈牙利）+ 最小点覆盖/最大独立集。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/graph/adjacency.hpp
 *
 * 复杂度：
 *   染色 O(V+E)；匈牙利 O(VE)。
 *
 * 用法（左部 1..nL，右部 1..nR，逻辑编号）：
 *   std::vector<int> matchR = max_matching(nL, nR, adjL);  // matchR[v]=左点，0=未匹配
 */

#include <vector>
#include <functional>
#include <queue>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/graph/adjacency.hpp"

namespace wbwlib {
namespace graph {

/**
 * @brief 二分图判定：BFS 黑白染色，同色邻边即非二分图。
 * @param g 无权邻接表（1 基）
 * @param color 输出参数，接收 color[1..n] ∈ {0,1,2}（0 = 未访问）
 * @return 可二染色返回 true，否则 false
 */
inline bool is_bipartite(const Adj& g, std::vector<int>& color) {
  int n = (int)g.size() - 1;
  color.assign(n + 1, 0);
  std::queue<int> q;
  for (int s = 1; s <= n; ++s)
    if (color[s] == 0) {
      color[s] = 1;
      q.push(s);
      while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v : g[u]) {
          if (color[v] == 0) {
            color[v] = 3 - color[u];
            q.push(v);
          } else if (color[v] == color[u]) {
            return false;
          }
        }
      }
    }
  return true;
}

/**
 * @brief 匈牙利算法求二分图最大匹配。
 * @param nL 左部点数（逻辑编号 1..nL）
 * @param nR 右部点数（逻辑编号 1..nR）
 * @param adjL adjL[u] 为左点 u 连接的右点集合（1 基）
 * @param matchR 可选输出参数，非空时接收 matchR[v] = 匹配的左点（0 = 未匹配）
 * @return 最大匹配大小
 */
inline int max_matching(int nL, int nR, const Adj& adjL,
                        std::vector<int>* matchR = nullptr) {
  std::vector<int> mL(nL + 1, 0), mR(nR + 1, 0);
  int ans = 0;
  std::vector<char> vis;
  std::function<bool(int)> aug = [&](int u) -> bool {
    for (int v : adjL[u]) {
      if (vis[v]) continue;
      vis[v] = 1;
      if (mR[v] == 0 || aug(mR[v])) {
        mR[v] = u;
        mL[u] = v;
        return true;
      }
    }
    return false;
  };
  for (int u = 1; u <= nL; ++u) {
    vis.assign(nR + 1, 0);
    if (aug(u)) ++ans;
  }
  if (matchR) *matchR = mR;
  return ans;
}

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_BIPARTITE_HPP