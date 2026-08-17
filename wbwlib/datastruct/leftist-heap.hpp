#ifndef WBWLIB_DS_LEFTIST_HEAP_HPP
#define WBWLIB_DS_LEFTIST_HEAP_HPP

/**
 * @file leftist-heap.hpp
 * @brief 左偏树（可并堆）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * 合并/插入/删除 O(log n)。
 *
 * @par 示例
 * @code{.cpp}
 *   using H = wbwlib::ds::LeftistHeap<int>;
 *   H h1, h2;
 *   int n1 = h1.push(3);          // 返回节点编号
 *   int n2 = h2.push(5);
 *   h1.merge(h2);                 // 把 h2 并入 h1
 *   int top = h1.top(); int v = h1.pop();
 * @endcode
 *
 * 说明：push 返回节点句柄，可配合 mark 删除指定节点（默认不实现删除指定节点）。
 */

#include <functional>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

/**
 * @brief 左偏树（可并堆），支持合并、插入、取堆顶、弹出堆顶。
 * @tparam T 元素类型
 * @tparam Cmp 比较器（默认 std::less<T>，即小根堆；根为最小元素）
 */
template<class T, class Cmp = std::less<T>>
class LeftistHeap {
  struct Node {
    T val;
    int l, r, dist;
  };
  static std::vector<Node> p_;   // 全局节点池（同类型所有堆共享），0 号哑节点
  int root_ = 0;
  Cmp cmp_;

  /// 合并两棵左偏树，返回新根（保证根为最值）
  int merge(int a, int b) {
    if (!a || !b) return a ? a : b;
    if (cmp_(p_[b].val, p_[a].val)) std::swap(a, b);   // 保证根是最小
    p_[a].r = merge(p_[a].r, b);
    if (p_[p_[a].l].dist < p_[p_[a].r].dist) std::swap(p_[a].l, p_[a].r);
    p_[a].dist = p_[p_[a].r].dist + 1;
    return a;
  }

 public:
  /**
   * @brief 插入一个值，返回节点编号（可用作句柄）。
   * @param v 待插入的值
   * @return 新节点的编号
   */
  int push(const T& v) {
    p_.push_back({v, 0, 0, 1});
    int n = (int)p_.size() - 1;
    root_ = merge(root_, n);
    return n;
  }

  /**
   * @brief 把堆 o 并入当前堆（o 被清空）。
   * @param o 待并入的堆（调用后为空）
   */
  void merge(LeftistHeap& o) {
    root_ = merge(root_, o.root_);
    o.root_ = 0;
  }

  /// 返回是否为空
  bool empty() const { return root_ == 0; }
  /// 返回是否非空（左偏树未维护 size 统计，仅指示 0/1）
  int size() const { return root_ == 0 ? 0 : 1; }

  /// 返回堆顶元素（要求非空）
  const T& top() const { return p_[root_].val; }

  /**
   * @brief 弹出堆顶元素并返回其值。
   * @return 被弹出的堆顶值
   */
  T pop() {
    T v = p_[root_].val;
    int nr = merge(p_[root_].l, p_[root_].r);
    root_ = nr;
    return v;
  }
};

} // namespace ds
} // namespace wbwlib

template<class T, class Cmp>
std::vector<typename wbwlib::ds::LeftistHeap<T, Cmp>::Node>
    wbwlib::ds::LeftistHeap<T, Cmp>::p_ = {{T(), 0, 0, 0}};

#endif // WBWLIB_DS_LEFTIST_HEAP_HPP