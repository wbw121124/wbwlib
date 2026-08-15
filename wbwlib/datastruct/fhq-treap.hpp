#ifndef WBWLIB_DS_FHQ_TREAP_HPP
#define WBWLIB_DS_FHQ_TREAP_HPP

/**
 * @file fhq-treap.hpp
 * @brief 无旋转 Treap（FHQ Treap）。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 两种形态：
 *  1. FHQTreap<T, Cmp> —— 平衡二叉搜索树（按关键值）：插入/删除/第 k 小/排名/是否存在；
 *  2. ImplicitTreap<T> —— 序列（隐式 Treap）：区间加、区间翻转、区间和、按位置插入删除。
 *
 * 复杂度：单次操作 O(log n) 期望。
 *
 * 用法：
 *   FHQTreap<int> tr; tr.insert(5); tr.kth(1); tr.erase(5);
 *   ImplicitTreap<i64> seq; seq.build({1,2,3}); seq.add(1,3,5); seq.reverse(1,2);
 */

#include <random>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

namespace details {

// 全局共享随机优先级（跨 TU 惰性初始化）
inline u32 randpri() {
  static std::mt19937_64 eng(0x3f3f3f5fULL ^ (u64)(std::uintptr_t)&eng);
  return (u32)(eng() >> 32);
}

} // namespace details

// ================= 权值模式 =================
template<class T, class Cmp = std::less<T>>
class FHQTreap {
  struct Node {
    T val;
    u32 pri;
    int sz, l, r;
  };

  std::vector<Node> pool_;   // arena 分配，避免大量 new/delete；节点下标从 1 起，0 为哑节点
  int root_ = 0;
  Cmp cmp_;

  int newnode(const T& v) {
    pool_.push_back({v, details::randpri(), 1, 0, 0});
    return (int)pool_.size() - 1;
  }
  void pull(int x) { pool_[x].sz = (x ? 1 : 0) + pool_[pool_[x].l].sz + pool_[pool_[x].r].sz; }

  /// 按关键值分裂：l 中全部 < key（按 cmp 由小到大），r 为其余
  void split(int x, const T& key, int& l, int& r) {
    if (!x) { l = r = 0; return; }
    if (cmp_(pool_[x].val, key)) {      // x 的值 < key → 在左侧集合
      split(pool_[x].r, key, pool_[x].r, r);
      l = x; pull(l);
    } else {
      split(pool_[x].l, key, l, pool_[x].l);
      r = x; pull(r);
    }
  }

  /// 按关键值分裂：l 中全部 <= key
  void split_le(int x, const T& key, int& l, int& r) {
    if (!x) { l = r = 0; return; }
    if (cmp_(pool_[x].val, key) || !cmp_(key, pool_[x].val)) {  // val <= key
      split_le(pool_[x].r, key, pool_[x].r, r);
      l = x; pull(l);
    } else {
      split_le(pool_[x].l, key, l, pool_[x].l);
      r = x; pull(r);
    }
  }

  int merge(int l, int r) {
    if (!l || !r) return l ? l : r;
    if (pool_[l].pri < pool_[r].pri) {
      pool_[l].r = merge(pool_[l].r, r);
      pull(l); return l;
    } else {
      pool_[r].l = merge(l, pool_[r].l);
      pull(r); return r;
    }
  }

  int kth(int x, int k) const {
    while (x) {
      int lsz = pool_[pool_[x].l].sz;
      if (k <= lsz) x = pool_[x].l;
      else if (k == lsz + 1) return x;
      else { k -= lsz + 1; x = pool_[x].r; }
    }
    return 0x3f3f3f3f;                 // 越界哨兵
  }

 public:
  FHQTreap() : cmp_() { pool_.push_back({T(), 0, 0, 0, 0}); }   // 0 号哑节点

  /// 插入一个值
  void insert(const T& v) {
    int a, b, n = newnode(v);
    split(root_, v, a, b);
    root_ = merge(merge(a, n), b);
  }

  /// 删除一个等于 v 的值；不存在时无操作
  void erase(const T& v) {
    int a, b, c;
    split_le(root_, v, a, b);        // a: <= v
    split(a, v, a, c);               // c: == v
    if (c) {
      // 取 c 的根节点移除：其左右子树（关键值仍等于 v）直接合并
      c = merge(pool_[c].l, pool_[c].r);
    }
    root_ = merge(merge(a, c), b);
  }

  bool has(const T& v) const {
    int x = root_;
    while (x) {
      if (cmp_(pool_[x].val, v)) x = pool_[x].r;
      else if (cmp_(v, pool_[x].val)) x = pool_[x].l;
      else return true;
    }
    return false;
  }

  /// 第 k 小（1 基）；k 越界返回哨兵（调用方自行断言）
  const T& kth(int k) const {
    int p = kth(root_, k);
    WBWLIB_ASSERT(p != 0x3f3f3f3f && "kth 越界");
    return pool_[p].val;
  }

  /// 严格小于 v 的元素个数（非破坏性遍历）
  int order_of_key(const T& v) const {
    int x = root_, cnt = 0;
    while (x) {
      if (cmp_(pool_[x].val, v)) {
        cnt += pool_[pool_[x].l].sz + 1;
        x = pool_[x].r;
      } else {
        x = pool_[x].l;
      }
    }
    return cnt;
  }

  int size() const { return pool_[root_].sz; }
  bool empty() const { return root_ == 0; }

  /// 中序遍历（输出有序序列），返回 vector
  std::vector<T> inorder() const {
    std::vector<T> res;
    std::vector<int> st;
    int x = root_;
    while (x || !st.empty()) {
      while (x) { st.push_back(x); x = pool_[x].l; }
      x = st.back(); st.pop_back();
      res.push_back(pool_[x].val);
      x = pool_[x].r;
    }
    return res;
  }
};

// ================= 隐式（序列）模式 =================
template<class T = i64>
class ImplicitTreap {
  struct Node {
    T val, sum, add;
    u32 pri;
    int sz, l, r;
    bool rev;
  };
  std::vector<Node> pool_;   // 0 号哑节点，实节点从 1 起
  int root_ = 0;

  int newnode(const T& v) {
    pool_.push_back({v, v, T(), details::randpri(), 1, 0, 0, false});
    return (int)pool_.size() - 1;
  }

  void apply_add(int x, const T& v) {
    if (!x) return;
    pool_[x].val += v;
    pool_[x].sum += v * T(pool_[x].sz);
    pool_[x].add += v;
  }
  void apply_rev(int x) {
    if (!x) return;
    pool_[x].rev = !pool_[x].rev;
    std::swap(pool_[x].l, pool_[x].r);
  }
  void push(int x) {
    if (!x) return;
    if (pool_[x].add != T()) {
      apply_add(pool_[x].l, pool_[x].add);
      apply_add(pool_[x].r, pool_[x].add);
      pool_[x].add = T();
    }
    if (pool_[x].rev) {
      apply_rev(pool_[x].l);
      apply_rev(pool_[x].r);
      pool_[x].rev = false;
    }
  }
  void pull(int x) {
    pool_[x].sz = 1 + pool_[pool_[x].l].sz + pool_[pool_[x].r].sz;
    pool_[x].sum = pool_[x].val + pool_[pool_[x].l].sum + pool_[pool_[x].r].sum;
  }

  /// 按大小分裂：l 含前 k 个，r 为剩余
  void split(int x, int k, int& l, int& r) {
    if (!x) { l = r = 0; return; }
    push(x);
    if (pool_[pool_[x].l].sz >= k) {
      split(pool_[x].l, k, l, pool_[x].l);
      r = x; pull(r);
    } else {
      split(pool_[x].r, k - pool_[pool_[x].l].sz - 1, pool_[x].r, r);
      l = x; pull(l);
    }
  }

  int merge(int l, int r) {
    if (!l || !r) return l ? l : r;
    if (pool_[l].pri < pool_[r].pri) {
      push(l);
      pool_[l].r = merge(pool_[l].r, r);
      pull(l); return l;
    } else {
      push(r);
      pool_[r].l = merge(l, pool_[r].l);
      pull(r); return r;
    }
  }

  void collect(int x, std::vector<T>& out) {
    if (!x) return;
    push(x);
    collect(pool_[x].l, out);
    out.push_back(pool_[x].val);
    collect(pool_[x].r, out);
  }

 public:
  ImplicitTreap() { pool_.push_back({}); }   // 0 号哑节点
  explicit ImplicitTreap(const std::vector<T>& a) : ImplicitTreap() { build(a); }

  /// 用数组构建（空则清空）
  void build(const std::vector<T>& a) {
    root_ = 0;
    for (const T& v : a) {
      int x = newnode(v);
      root_ = merge(root_, x);
    }
  }

  int size() const { return pool_[root_].sz; }
  bool empty() const { return root_ == 0; }

  /// 在位置 pos（1 基，1 表示最前，size+1 表示最后）前插入 v
  void insert(int pos, const T& v) {
    int a, b, n = newnode(v);
    split(root_, pos - 1, a, b);
    root_ = merge(merge(a, n), b);
  }

  void push_back(const T& v) { insert(size() + 1, v); }

  /// 删除位置 pos（1 基）
  void erase(int pos) {
    int a, b, c;
    split(root_, pos - 1, a, b);
    split(b, 1, b, c);
    root_ = merge(a, c);
  }

  /// 区间 [l, r]（1 基闭区间）加 v
  void add(int l, int r, const T& v) {
    int a, b, c;
    split(root_, l - 1, a, b);
    split(b, r - l + 1, b, c);
    apply_add(b, v);
    root_ = merge(merge(a, b), c);
  }

  /// 区间 [l, r] 翻转
  void reverse(int l, int r) {
    int a, b, c;
    split(root_, l - 1, a, b);
    split(b, r - l + 1, b, c);
    apply_rev(b);
    root_ = merge(merge(a, b), c);
  }

  /// 区间和
  T sum(int l, int r) {
    int a, b, c;
    split(root_, l - 1, a, b);
    split(b, r - l + 1, b, c);
    T s = pool_[b].sum;
    root_ = merge(merge(a, b), c);
    return s;
  }

  /// 单点值（位置 pos，1 基）；越界触发断言
  T get(int pos) {
    int x = root_;
    int k = pos;
    while (x) {
      push(x);
      int lsz = pool_[pool_[x].l].sz;
      if (k == lsz + 1) return pool_[x].val;
      if (k <= lsz) x = pool_[x].l;
      else { k -= lsz + 1; x = pool_[x].r; }
    }
    WBWLIB_ASSERT(false && "get 越界");
    return T();
  }

  /// 顺序输出整个序列
  std::vector<T> to_vector() {
    std::vector<T> out;
    collect(root_, out);
    return out;
  }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_FHQ_TREAP_HPP