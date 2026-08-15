#ifndef WBWLIB_GRAPH_VIRTUAL_TREE_HPP
#define WBWLIB_GRAPH_VIRTUAL_TREE_HPP

/**
 * @file virtual-tree.hpp
 * @brief 虚树：保留关键点及其两两 LCA，压缩成 O(k) 规模的树。
 *
 * 依赖：wbwlib/core/base.hpp、wbwlib/graph/lca.hpp
 *
 * 复杂度：O(k log n)。
 *
 * 用法：
 *   wbwlib::graph::VirtualTree vt(lc);          // lc 为已建好的 LCA（含 dfn）
 *   vt.build(keys);                              // keys 为 1 基点集
 *   vt.edges                                    // (u,v) 原始编号边列表
 *   vt.nodes                                    // 虚树包含点的原始编号
 */

#include <vector>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/graph/lca.hpp"

namespace wbwlib {
namespace graph {

class VirtualTree {
 public:
  const LCA* lc_;
  std::vector<int> nodes;              ///< 虚树顶点（原始编号，dfn 升序）
  std::vector<std::pair<int, int>> edges;  ///< 无向边

  explicit VirtualTree(const LCA& lc) : lc_(&lc) {}

  void build(const std::vector<int>& keys) {
    const LCA& lc = *lc_;
    nodes.clear();
    edges.clear();
    nodes = keys;
    std::sort(nodes.begin(), nodes.end(),
              [&](int a, int b) { return lc.tin[a] < lc.tin[b]; });
    nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());
    int k = (int)nodes.size();
    for (int i = k - 1; i >= 1; --i) nodes.push_back(lc.query(nodes[i - 1], nodes[i]));
    std::sort(nodes.begin(), nodes.end(),
              [&](int a, int b) { return lc.tin[a] < lc.tin[b]; });
    nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());
    // 经典栈式建树
    std::vector<int> st;
    st.reserve(nodes.size());
    for (int u : nodes) {
      if (st.empty()) {
        st.push_back(u);
        continue;
      }
      int l = lc.query(u, st.back());
      if (l == st.back()) {
        edges.push_back({st.back(), u});
        st.push_back(u);
        continue;
      }
      while ((int)st.size() >= 2 && lc.tin[st[(int)st.size() - 2]] >= lc.tin[l]) {
        edges.push_back({st[(int)st.size() - 2], st.back()});
        st.pop_back();
      }
      if (st.back() != l) {
        edges.push_back({l, st.back()});
        st.back() = l;
      }
      edges.push_back({l, u});
      st.push_back(u);
    }
    while ((int)st.size() >= 2) {
      edges.push_back({st[(int)st.size() - 2], st.back()});
      st.pop_back();
    }
  }
};

} // namespace graph
} // namespace wbwlib

#endif // WBWLIB_GRAPH_VIRTUAL_TREE_HPP