#ifndef WBWLIB_GRAPH_VIRTUAL_TREE_HPP
#define WBWLIB_GRAPH_VIRTUAL_TREE_HPP

/**
 * @file virtual-tree.hpp
 * @brief 虚树：保留关键点及其两两 LCA，压缩成 O(k) 规模的树。
 *
 * @par 依赖
 * wbwlib/core/base.hpp、wbwlib/graph/lca.hpp
 *
 * @par 复杂度
 * O(k log n)。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::graph::VirtualTree vt(lc);          // lc 为已建好的 LCA（含 dfn）
 *   vt.build(keys);                              // keys 为 1 基点集
 *   vt.edges                                    // (u,v) 原始编号边列表
 *   vt.nodes                                    // 虚树包含点的原始编号
 * @endcode
 */

#include <vector>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/graph/lca.hpp"

namespace wbwlib {
namespace graph {

/**
 * @brief 虚树：保留关键点及其两两 LCA，压缩成 O(k) 规模的树。
 */
class VirtualTree {
 public:
  const LCA* lc_;
  std::vector<int> nodes;              ///< 虚树顶点（原始编号，dfn 升序）
  std::vector<std::pair<int, int>> edges;  ///< 无向边

  /**
   * @brief 以已构建的 LCA 初始化虚树构建器。
   * @param lc 已建好的 LCA 对象（含 DFS 序）
   */
  explicit VirtualTree(const LCA& lc) : lc_(&lc) {}

  /**
   * @brief 由关键点集构建虚树（栈式建树，自动去重）。
   * @param keys 关键点集（1 基原始编号）
   */
  void build(const std::vector<int>& keys) {
    const LCA& lc = *lc_;
    nodes.clear();
    edges.clear();
    std::vector<int> ks = keys;
    std::sort(ks.begin(), ks.end(),
              [&](int a, int b) { return lc.tin[a] < lc.tin[b]; });
    ks.erase(std::unique(ks.begin(), ks.end()), ks.end());
    int k = (int)ks.size();
    for (int i = k - 1; i >= 1; --i) ks.push_back(lc.query(ks[i - 1], ks[i]));
    std::sort(ks.begin(), ks.end(),
              [&](int a, int b) { return lc.tin[a] < lc.tin[b]; });
    ks.erase(std::unique(ks.begin(), ks.end()), ks.end());
    nodes = ks;
    // 栈式建树：弹出时不连边，只在补齐 LCA 层与挂新点时连，最后统一去重
    std::vector<std::pair<int, int>> es;
    auto add = [&](int a, int b) {
      if (a == b) return;
      es.push_back({a, b});
    };
    std::vector<int> st;
    st.reserve(nodes.size());
    for (int u : nodes) {
      if (st.empty()) {
        st.push_back(u);
        continue;
      }
      int l = lc.query(u, st.back());
      while ((int)st.size() >= 2 && lc.tin[st[(int)st.size() - 2]] >= lc.tin[l])
        st.pop_back();
      if (st.back() != l) {
        add(l, st.back());
        st.back() = l;
      }
      add(l, u);
      st.push_back(u);
    }
    std::sort(es.begin(), es.end());
    es.erase(std::unique(es.begin(), es.end()), es.end());
    edges = es;
  }
};

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_VIRTUAL_TREE_HPP