#ifndef WBWLIB_DS_LEFTIST_HEAP_HPP
#define WBWLIB_DS_LEFTIST_HEAP_HPP

/**
 * @file leftist-heap.hpp
 * @brief 左偏树（可并堆）。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：合并/插入/删除 O(log n)。
 *
 * 用法：
 *   using H = wbwlib::ds::LeftistHeap<int>;
 *   H h1, h2;
 *   int n1 = h1.push(3);          // 返回节点编号
 *   int n2 = h2.push(5);
 *   h1.merge(h2);                 // 把 h2 并入 h1
 *   int top = h1.top(); int v = h1.pop();
 *
 * 说明：push 返回节点句柄，可配合 mark 删除指定节点（默认不实现删除指定节点）。
 */

#include <functional>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

template<class T, class Cmp = std::less<T>>
class LeftistHeap {
  struct Node {
    T val;
    int l, r, dist;
  };
  std::vector<Node> p_;   // 0 号哑节点（dist=0），实节点从 1 起
  int root_ = 0;
  Cmp cmp_;

  int merge(int a, int b) {
    if (!a || !b) return a ? a : b;
    if (cmp_(p_[b].val, p_[a].val)) std::swap(a, b);   // 保证根是最小
    p_[a].r = merge(p_[a].r, b);
    if (p_[p_[a].l].dist < p_[p_[a].r].dist) std::swap(p_[a].l, p_[a].r);
    p_[a].dist = p_[p_[a].r].dist + 1;
    return a;
  }

 public:
  LeftistHeap() { p_.push_back({T(), 0, 0, 0}); }   // 0 号哑节点

  int push(const T& v) {
    p_.push_back({v, 0, 0, 1});
    int n = (int)p_.size() - 1;
    root_ = merge(root_, n);
    return n;
  }

  void merge(LeftistHeap& o) {
    root_ = merge(root_, o.root_);
    o.root_ = 0;
  }

  bool empty() const { return root_ == 0; }
  int size() const { return root_ == 0 ? 0 : 1; }   // 仅指示非空（左偏树无 size 统计）

  const T& top() const { return p_[root_].val; }

  /// 弹出堆顶并返回值
  T pop() {
    T v = p_[root_].val;
    int nr = merge(p_[root_].l, p_[root_].r);
    root_ = nr;
    return v;
  }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_LEFTIST_HEAP_HPP