#ifndef WBWLIB_DS_DSU_HPP
#define WBWLIB_DS_DSU_HPP

/**
 * @file dsu.hpp
 * @brief 并查集。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 提供三类：
 *  1. DSU        —— 路径压缩 + 按秩合并（可维护集合大小/元素个数）；
 *  2. WeightedDSU —— 带权并查集（维护到根的权值差，支持模运算环）；
 *  3. RollbackDSU —— 可撤销并查集（按大小合并 + 栈式回滚，配合线段树分治）。
 *
 * 复杂度：DSU 单次操作 O(α(n))；RollbackDSU O(log n)。
 *
 * 用法：
 *   wbwlib::ds::DSU d(n);
 *   d.unite(a, b); d.find(a) == d.find(b);
 *   wbwlib::ds::WeightedDSU w(n);        // w.unite(a,b, d) 表示 a 到 b 权差为 d
 *   wbwlib::ds::RollbackDSU r(n);
 *   int op = r.unite(a,b); r.rollback(op);
 */

#include <functional>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

// ================= 标准并查集 =================
class DSU {
 public:
  std::vector<int> fa_, rk_;   ///< 父节点与秩

  explicit DSU(int n) { init(n); }

  void init(int n) {
    fa_.resize(n + 1);
    rk_.assign(n + 1, 1);
    for (int i = 1; i <= n; ++i) fa_[i] = i;
  }

  int find(int x) {
    while (fa_[x] != x) x = fa_[x] = fa_[fa_[x]];   // 路径压缩
    return x;
  }

  /// 合并，返回是否发生实际合并
  bool unite(int a, int b) {
    a = find(a); b = find(b);
    if (a == b) return false;
    if (rk_[a] < rk_[b]) std::swap(a, b);
    fa_[b] = a;
    if (rk_[a] == rk_[b]) ++rk_[a];
    return true;
  }

  bool same(int a, int b) { return find(a) == find(b); }

  /// 查询集合大小（find 后集合代表元的 rk 为树高，非大小；提供独立 sz 版本见 DSUWave）
  int rank_of(int x) const { return rk_[x]; }
};

/// 支持维护连通块大小的并查集
class DSUWave : public DSU {
  std::vector<int> sz_;

 public:
  explicit DSUWave(int n) : DSU(n) {
    sz_.assign(n + 1, 1);
  }

  int find(int x) {
    while (fa_[x] != x) x = fa_[x] = fa_[fa_[x]];
    return x;
  }

  bool unite(int a, int b) {
    a = find(a); b = find(b);
    if (a == b) return false;
    if (rk_[a] < rk_[b]) std::swap(a, b);
    fa_[b] = a;
    sz_[a] += sz_[b];
    if (rk_[a] == rk_[b]) ++rk_[a];
    return true;
  }

  int size_of(int x) { return sz_[find(x)]; }
};

// ================= 带权并查集 =================
/**
 * 维护 xi - x_root 的关系；支持加权合并。
 * 用法：uniteIf(a, b, d, mod) 表示 a 与 b 的权差为 d（a - b ≡ d (mod mod)，可选）。
 * 无模：uniteD(a, b, d) 表示 value(a) = value(b) + d。
 */
class WeightedDSU {
  std::vector<int> fa_;
  std::vector<i64> wt_;    ///< wt_[x] = value(x) - value(fa_[x])
  bool mod_mode_;
  i64 mod_;

 public:
  /// mode=true 表示模运算（值域 [0, mod)），否则整数值域
  explicit WeightedDSU(int n, bool mod = true, i64 modulus = 0)
      : mod_mode_(mod), mod_(modulus) {
    fa_.resize(n + 1);
    wt_.assign(n + 1, 0);
    for (int i = 1; i <= n; ++i) fa_[i] = i;
  }

  int find(int x) {
    if (fa_[x] == x) return x;
    int r = find(fa_[x]);
    wt_[x] += wt_[fa_[x]];          // 路径压缩累加权值
    fa_[x] = r;
    return r;
  }

  /// 返回 value(x)（以根为基准的值）
  i64 to_root(int x) { find(x); return wt_[x]; }

  /// 合并：建立 value(a) 与 value(b) 的关系 value(b) - value(a) = diff
  /// 返回是否与已有约束一致
  bool unite(int a, int b, i64 diff) {
    int ra = find(a), rb = find(b);
    if (ra == rb) {
      i64 cur = to_root(b) - to_root(a);
      if (mod_mode_) cur = norm(cur);
      return cur == (mod_mode_ ? norm(diff) : diff);
    }
    // diff = value(b) - value(a) = (wt_b + value(rb)) - (wt_a + value(ra))
    // 令 value(ra) = value(rb) + (wt_b + value(rb)... ) 化解：
    i64 t = to_root(b) - to_root(a) - diff;
    if (mod_mode_) t = norm(t);
    fa_[ra] = rb;
    wt_[ra] = t;                    // value(ra) = value(rb) + t
    return true;
  }

  bool same(int a, int b) { return find(a) == find(b); }

  i64 diff(int a, int b) {          // value(b) - value(a)
    find(a); find(b);
    i64 d = to_root(b) - to_root(a);
    return mod_mode_ ? norm(d) : d;
  }

 private:
  i64 norm(i64 x) const {
    x %= mod_;
    return x < 0 ? x + mod_ : x;
  }
};

// ================= 可撤销并查集 =================
class RollbackDSU {
  std::vector<int> fa_, sz_;
  struct Op { int u, v, szu; };     // 合并前记录
  std::vector<Op> hist_;

  int find(int x) const {
    while (fa_[x] != x) x = fa_[x];
    return x;
  }

 public:
  explicit RollbackDSU(int n) : fa_(n + 1), sz_(n + 1, 1) {
    for (int i = 1; i <= n; ++i) fa_[i] = i;
  }

  bool unite(int a, int b) {        // 按大小合并（不做路径压缩）
    int ra = find(a), rb = find(b);
    if (ra == rb) return false;
    if (sz_[ra] < sz_[rb]) std::swap(ra, rb);
    hist_.push_back({rb, ra, sz_[ra]});
    fa_[rb] = ra;
    sz_[ra] += sz_[rb];
    return true;
  }

  bool same(int a, int b) const { return find(a) == find(b); }

  /// 回滚最近的一次 unite，返回该次数码
  void rollback() {
    WBWLIB_ASSERT(!hist_.empty());
    Op o = hist_.back(); hist_.pop_back();
    fa_[o.u] = o.u;
    sz_[o.v] = o.szu;
  }

  int snapshot() const { return (int)hist_.size(); }
  void rollback_to(int snap) {
    while ((int)hist_.size() > snap) rollback();
  }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_DSU_HPP