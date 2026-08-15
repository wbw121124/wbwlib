#ifndef WBWLIB_DS_SPLAY_HPP
#define WBWLIB_DS_SPLAY_HPP

/**
 * @file splay.hpp
 * @brief 伸展树 Splay（序列模式，文艺平衡树）。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：单次操作均摊 O(log n)。支持区间翻转、区间加、区间和、按位置插入/删除。
 *
 * 区间操作实现：把端点节点伸展到根/右子后提取中段子树，操作后再接回。
 *
 * 用法：
 *   wbwlib::ds::Splay<i64> sp({1,2,3});
 *   sp.reverse(1, 3); sp.add(1, 2, 5);
 *   sp.insert_at(2, 9); sp.erase(1);
 *   sp.sum(1, 3); auto v = sp.to_vector();
 */

#include <functional>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

template<class T = i64>
class Splay {
  struct Node {
    int ch[2], fa, sz;
    T val, sum, add;
    bool rev;
  };
  std::vector<Node> p_;   // 0 号哑节点，实节点从 1 起
  int root_ = 0;

  int newnode(const T& v) {
    p_.push_back({ {0, 0}, 0, 1, v, v, T(), false });
    return (int)p_.size() - 1;
  }

  // ---------- 懒标记 ----------
  void push(int x) {
    if (!x) return;
    if (p_[x].rev) {
      std::swap(p_[x].ch[0], p_[x].ch[1]);
      if (p_[x].ch[0]) p_[p_[x].ch[0]].rev ^= 1;
      if (p_[x].ch[1]) p_[p_[x].ch[1]].rev ^= 1;
      p_[x].rev = false;
    }
    if (p_[x].add != T()) {
      T a = p_[x].add;
      for (int c = 0; c < 2; ++c) if (p_[x].ch[c]) {
        int u = p_[x].ch[c];
        p_[u].val += a;
        p_[u].sum += a * T(p_[u].sz);
        p_[u].add += a;
      }
      p_[x].add = T();
    }
  }

  void pull(int x) {
    p_[x].sz = 1 + p_[p_[x].ch[0]].sz + p_[p_[x].ch[1]].sz;
    p_[x].sum = p_[x].val + p_[p_[x].ch[0]].sum + p_[p_[x].ch[1]].sum;
  }

  // ---------- 旋转 / 伸展 ----------
  void rotate(int x) {
    int y = p_[x].fa, z = p_[y].fa;
    int k = (p_[y].ch[1] == x);
    int b = p_[x].ch[k ^ 1];
    if (z) p_[z].ch[p_[z].ch[1] == y] = x;
    p_[x].fa = z;
    p_[x].ch[k ^ 1] = y; p_[y].fa = x;
    p_[y].ch[k] = b; if (b) p_[b].fa = y;
    pull(y); pull(x);
  }

  void splay(int x) {
    while (p_[x].fa) {
      int y = p_[x].fa, z = p_[y].fa;
      if (z) {
        if ((p_[y].ch[0] == x) == (p_[z].ch[0] == y)) rotate(y);
        else rotate(x);
      }
      rotate(x);
    }
    root_ = x;
  }

  /// 按位置 kth（1 基），返回节点并伸展到根
  int kth(int k) {
    int x = root_;
    while (true) {
      push(x);
      int lsz = p_[p_[x].ch[0]].sz;
      if (k == lsz + 1) break;
      if (k <= lsz) x = p_[x].ch[0];
      else k -= lsz + 1, x = p_[x].ch[1];
    }
    splay(x);
    return x;
  }

  // ---------- 区间提取 ----------
  /**
   * 将 [l, r] 对应的子树分离出来作为独立的平衡树返回。
   * 提取后该子树从原树断开（root 为整体树根，父节点已脱开）。
   * 之后可用 restore 接回。
   */
  struct Interval {
    int tree;         ///< 区间子树根
    int parent;       ///< 其父节点（原树根）
    int side;         ///< parent->ch[side] 原指向 tree
  };

  Interval cut(int l, int r) {
    int n = size();
    if (l == 1 && r == n) return {root_, 0, -1};
    Interval iv;
    if (r < n) {
      if (l > 1) kth(l - 1);
      kth(r + 1);
      iv.parent = root_;
      iv.side = 0;
    } else {                      // r == n
      kth(l - 1);
      iv.parent = root_;
      iv.side = 1;
    }
    iv.tree = p_[iv.parent].ch[iv.side];
    p_[iv.parent].ch[iv.side] = 0;
    if (iv.tree) p_[iv.tree].fa = 0;
    pull(iv.parent);
    return iv;
  }

  void restore(const Interval& iv) {
    if (iv.parent == 0) { root_ = iv.tree; return; }
    p_[iv.parent].ch[iv.side] = iv.tree;
    if (iv.tree) p_[iv.tree].fa = iv.parent;
    pull(iv.parent);
    root_ = iv.parent;
  }

 public:
  Splay() { p_.push_back({}); }   // 0 号哑节点（sz=0, sum=0）
  explicit Splay(const std::vector<T>& a) : Splay() { build(a); }

  void build(const std::vector<T>& a) {
    root_ = 0;
    if (a.empty()) return;
    // 平衡建树：取中点递归
    std::function<int(int, int, int)> rec = [&](int l, int r, int fa) -> int {
      if (l > r) return 0;
      int mid = (l + r) >> 1;
      int x = newnode(a[mid]);
      p_[x].fa = fa;
      // 注意：rec 内 newnode 会使 p_ 重分配，必须先拿到返回值再按下标赋值（避免悬空引用）
      int cl = rec(l, mid - 1, x);
      int cr = rec(mid + 1, r, x);
      p_[x].ch[0] = cl;
      p_[x].ch[1] = cr;
      pull(x);
      return x;
    };
    root_ = rec(0, (int)a.size() - 1, 0);
  }

  int size() const { return p_[root_].sz; }
  bool empty() const { return root_ == 0; }

  /// 在位置 k（1 基）前插入；k == size()+1 表示尾部
  void insert(int k, const T& v) {
    int n = size();
    int x = newnode(v);
    if (n == 0) { root_ = x; return; }
    if (k == 1) {
      p_[x].ch[1] = root_; p_[root_].fa = x;
      pull(x); root_ = x; return;
    }
    if (k == n + 1) {
      kth(n);
      p_[root_].ch[1] = x; p_[x].fa = root_;
      pull(root_); return;
    }
    kth(k);                       // 根为第 k 个；左子树前 k-1 个
    int L = p_[root_].ch[0];
    if (L) p_[L].fa = 0;
    p_[root_].ch[0] = x;
    p_[x].fa = root_;
    p_[x].ch[0] = L;
    if (L) p_[L].fa = x;
    pull(x); pull(root_);
  }

  void push_back(const T& v) { insert(size() + 1, v); }

  /// 删除位置 k
  void erase(int k) {
    int n = size();
    if (n == 0) return;
    if (n == 1) { root_ = 0; return; }
    kth(k);
    int L = p_[root_].ch[0], R = p_[root_].ch[1];
    if (L) p_[L].fa = 0;
    if (R) p_[R].fa = 0;
    if (!L) { root_ = R; return; }
    if (!R) { root_ = L; return; }
    int m = L;
    while (p_[m].ch[1]) m = p_[m].ch[1];     // L 中最大
    while (p_[m].fa) rotate(m);              // 旋为 L 子树的根
    p_[m].ch[1] = R; p_[R].fa = m;
    pull(m);
    root_ = m;
  }

  /// 翻转 [l, r]
  void reverse(int l, int r) {
    if (l >= r) return;
    Interval iv = cut(l, r);
    p_[iv.tree].rev ^= 1;
    restore(iv);
  }

  /// 区间加
  void add(int l, int r, const T& v) {
    Interval iv = cut(l, r);
    p_[iv.tree].val += v;
    p_[iv.tree].sum += v * T(p_[iv.tree].sz);
    p_[iv.tree].add += v;
    restore(iv);
  }

  /// 区间和
  T sum(int l, int r) {
    Interval iv = cut(l, r);
    T s = p_[iv.tree].sum;
    restore(iv);
    return s;
  }

  /// 单点值
  T get(int k) { kth(k); return p_[root_].val; }

  std::vector<T> to_vector() {
    std::vector<T> out;
    std::vector<int> st;
    int x = root_;
    while (x || !st.empty()) {
      while (x) {
        push(x);
        st.push_back(x);
        x = p_[x].ch[0];
      }
      x = st.back(); st.pop_back();
      out.push_back(p_[x].val);
      x = p_[x].ch[1];
    }
    return out;
  }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_SPLAY_HPP