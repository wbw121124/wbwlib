#ifndef WBWLIB_DS_CARTESIAN_TREE_HPP
#define WBWLIB_DS_CARTESIAN_TREE_HPP

/**
 * @file cartesian-tree.hpp
 * @brief 笛卡尔树：中序遍历为原序列、满足堆性质的二叉树。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：O(n)（单调栈）。
 *
 * 用法：构建小根笛卡尔树（值越小越接近根）：
 *   auto [root, l, r] = wbwlib::ds::cartesian_tree(a, false);
 *   // l[i], r[i] 为 i 的左右孩子（0 表示无）；root 为根
 */

#include <functional>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

struct CartesianTree {
  int root = 0;              ///< 根的下标（1 基）
  std::vector<int> l, r, fa; ///< 左孩子、右孩子、父节点（无则 0）
};

/// a 为 1 基（a[0] 占位）。is_max=true 建大根笛卡尔树，否则小根。
template<class T, class Cmp = std::less<T>>
inline CartesianTree cartesian_tree(const std::vector<T>& a, bool is_max = false) {
  int n = (int)a.size() - 1;
  CartesianTree ct;
  ct.l.assign(n + 1, 0);
  ct.r.assign(n + 1, 0);
  ct.fa.assign(n + 1, 0);
  std::vector<int> st;
  for (int i = 1; i <= n; ++i) {
    int last = 0;
    Cmp cmp;                    // Cmp：st.back() 比 i "更靠顶" 时弹出
    while (!st.empty()) {
      bool popit;
      if (is_max)       popit = cmp(a[st.back()], a[i]);   // 大根：栈顶值 < i 的值
      else              popit = a[st.back()] > a[i];       // 小根：栈顶值 > i 的值
      if (!popit) break;
      last = st.back();
      st.pop_back();
    }
    if (!st.empty()) {
      ct.fa[i] = st.back();
      ct.r[st.back()] = i;
    }
    ct.l[i] = last;
    if (last) ct.fa[last] = i;
    st.push_back(i);
  }
  ct.root = st[0];
  for (int x : st) {
    if (x != ct.root && ct.fa[x] == 0) ct.fa[x] = st[0];
  }
  return ct;
}

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_CARTESIAN_TREE_HPP