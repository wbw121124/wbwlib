#ifndef WBWLIB_GRAPH_TREE_DP_HPP
#define WBWLIB_GRAPH_TREE_DP_HPP

/**
 * @file tree-dp.hpp
 * @brief 树上 DP 套路：直径、中心、换根 DP 模板。
 *
 * 依赖：wbwlib/core/base.hpp、wbwlib/graph/adjacency.hpp
 *
 * 复杂度：O(n)。
 *
 * 换根 DP（reroot_dp）约定：
 *   T identity                    一字聚合单位（如 {0} / 0 …）
 *   Merge merge(a,b)              子树贡献合并（要求可结合，顺序固定）
 *   Push push(up, u, v)           由「u 侧除 v 分支外的整树贡献 up」
 *                                 推出发给子树 v 的贡献
 *   f[u] 输出：以 u 为根整棵树的聚合值。
 */

#include <vector>
#include <queue>
#include <functional>
#include <algorithm>
#include <tuple>
#include "wbwlib/core/base.hpp"
#include "wbwlib/graph/adjacency.hpp"

namespace wbwlib {
namespace graph {

/// 树直径：返回 {直径长度, 端点1, 端点2}（无权边，长度为边数）
inline std::tuple<int, int, int> tree_diameter(const Adj& g) {
  int n = (int)g.size() - 1;
  std::function<std::pair<int, int>(int)> far = [&](int s) {
    std::vector<int> dep(n + 1, -1);
    std::queue<int> q;
    dep[s] = 0;
    q.push(s);
    int best = s;
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (int v : g[u])
        if (dep[v] < 0) {
          dep[v] = dep[u] + 1;
          q.push(v);
          if (dep[v] > dep[best]) best = v;
        }
    }
    return std::make_pair(best, dep[best]);
  };
  int a = far(1).first;
  auto r = far(a);
  return std::make_tuple(r.second, a, r.first);
}

/// 树中心：使 max(最大子树, 其余点数) 最小的点（1~2 个）
inline std::vector<int> tree_centers(const Adj& g) {
  int n = (int)g.size() - 1;
  std::vector<int> sz(n + 1, 0), fa(n + 1, 0);
  std::vector<int> order;
  order.reserve(n);
  std::function<void(int)> dfs = [&](int u) {
    order.push_back(u);
    for (int v : g[u])
      if (v != fa[u]) {
        fa[v] = u;
        dfs(v);
      }
  };
  dfs(1);
  std::vector<int> mx(n + 1, 0);
  for (int i = n - 1; i >= 0; --i) ++sz[order[i]];
  int best = n + 1;
  for (int i = 0; i < n; ++i) {
    int u = order[i];
    int cur = n - sz[u];
    for (int v : g[u])
      if (v != fa[u]) cur = std::max(cur, sz[v]);
    mx[u] = cur;
    best = std::min(best, cur);
  }
  std::vector<int> res;
  for (int u = 1; u <= n; ++u)
    if (mx[u] == best) res.push_back(u);
  return res;
}

/**
 * 换根 DP 通用模板。
 * 内部：第一次 DFS 挂树并求 dp[u]=merge(子们)；
 *       第二次 DFS 沿 down 传递父侧贡献，用前后缀 O(1) 剔除单条子树。
 */
template<class T, class Merge, class Push>
std::vector<T> reroot_dp(int n, const Adj& g, int root,
                         T identity, Merge merge, Push push) {
  std::vector<int> fa(n + 1, 0);
  std::vector<std::vector<int>> ch(n + 1);
  std::vector<int> order;
  order.reserve(n);
  std::function<void(int)> hang = [&](int u) {
    order.push_back(u);
    for (int v : g[u])
      if (v != fa[u]) {
        fa[v] = u;
        ch[u].push_back(v);
        hang(v);
      }
  };
  hang(root);

  std::vector<T> dp(n + 1);
  for (int i = n - 1; i >= 0; --i) {
    int u = order[i];
    T acc = identity;
    for (int v : ch[u]) acc = merge(acc, dp[v]);
    dp[u] = acc;
  }

  std::vector<T> f(n + 1);
  std::function<void(int, T)> down = [&](int u, T up) {
    f[u] = merge(up, dp[u]);
    int k = (int)ch[u].size();
    // 前缀/后缀
    std::vector<T> pre(k), suf(k);
    for (int i = 0; i < k; ++i)
      pre[i] = merge(i ? pre[i - 1] : identity, dp[ch[u][i]]);
    for (int i = k - 1; i >= 0; --i)
      suf[i] = merge(i + 1 < k ? suf[i + 1] : identity, dp[ch[u][i]]);
    for (int i = 0; i < k; ++i) {
      T rest = merge(up, merge(i ? pre[i - 1] : identity,
                               i + 1 < k ? suf[i + 1] : identity));
      down(ch[u][i], push(rest, u, ch[u][i]));
    }
  };
  down(root, identity);
  return f;
}

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_TREE_DP_HPP