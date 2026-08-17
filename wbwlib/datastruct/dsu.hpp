#ifndef WBWLIB_DS_DSU_HPP
#define WBWLIB_DS_DSU_HPP

/**
 * @file dsu.hpp
 * @brief 并查集。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * 提供三类：
 *  1. DSU        —— 路径压缩 + 按秩合并（可维护集合大小/元素个数）；
 *  2. WeightedDSU —— 带权并查集（维护到根的权值差，支持模运算环）；
 *  3. RollbackDSU —— 可撤销并查集（按大小合并 + 栈式回滚，配合线段树分治）。
 *
 * @par 复杂度
 * DSU 单次操作 O(α(n))；RollbackDSU O(log n)。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::ds::DSU d(n);
 *   d.unite(a, b); d.find(a) == d.find(b);
 *   wbwlib::ds::WeightedDSU w(n);        // w.unite(a,b, d) 表示 a 到 b 权差为 d
 *   wbwlib::ds::RollbackDSU r(n);
 *   int op = r.unite(a,b); r.rollback(op);
 * @endcode
 */

#include <functional>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

// ================= 标准并查集 =================
/**
 * @brief 标准并查集：路径压缩 + 按秩合并。
 */
class DSU {
 public:
  std::vector<int> fa_, rk_;   ///< 父节点与秩

  /**
   * @brief 构造 n 个元素的并查集（每个元素自成一集合）。
   * @param n 元素个数（编号 1..n）
   */
  explicit DSU(int n) { init(n); }

  /**
   * @brief 重置为 n 个元素的并查集。
   * @param n 元素个数（编号 1..n）
   */
  void init(int n) {
    fa_.resize(n + 1);
    rk_.assign(n + 1, 1);
    for (int i = 1; i <= n; ++i) fa_[i] = i;
  }

  /**
   * @brief 查询 x 所在集合的代表元（路径压缩）。
   * @param x 元素编号
   * @return x 所在集合的代表元
   */
  int find(int x) {
    while (fa_[x] != x) x = fa_[x] = fa_[fa_[x]];   // 路径压缩
    return x;
  }

  /**
   * @brief 合并 a、b 所在集合（按秩合并）。
   * @param a 元素编号
   * @param b 元素编号
   * @return 是否发生了实际合并（原本不在同一集合返回 true）
   */
  bool unite(int a, int b) {
    a = find(a); b = find(b);
    if (a == b) return false;
    if (rk_[a] < rk_[b]) std::swap(a, b);
    fa_[b] = a;
    if (rk_[a] == rk_[b]) ++rk_[a];
    return true;
  }

  /**
   * @brief 判断 a、b 是否在同一集合。
   * @param a 元素编号
   * @param b 元素编号
   * @return 同一集合返回 true
   */
  bool same(int a, int b) { return find(a) == find(b); }

  /**
   * @brief 查询 x 的秩（树的近似高度，非集合大小）。
   * @param x 元素编号
   * @return rk_[x]；注意集合大小请用 DSUWave
   */
  int rank_of(int x) const { return rk_[x]; }
};

/**
 * @brief 维护连通块大小的并查集（路径压缩 + 按秩合并）。
 */
class DSUWave : public DSU {
  std::vector<int> sz_;

 public:
  /**
   * @brief 构造 n 个元素的并查集。
   * @param n 元素个数（编号 1..n）
   */
  explicit DSUWave(int n) : DSU(n) {
    sz_.assign(n + 1, 1);
  }

  /**
   * @brief 查询 x 所在集合的代表元（路径压缩）。
   * @param x 元素编号
   * @return x 所在集合的代表元
   */
  int find(int x) {
    while (fa_[x] != x) x = fa_[x] = fa_[fa_[x]];
    return x;
  }

  /**
   * @brief 合并 a、b 所在集合，并同步维护集合大小。
   * @param a 元素编号
   * @param b 元素编号
   * @return 是否发生了实际合并（原本不在同一集合返回 true）
   */
  bool unite(int a, int b) {
    a = find(a); b = find(b);
    if (a == b) return false;
    if (rk_[a] < rk_[b]) std::swap(a, b);
    fa_[b] = a;
    sz_[a] += sz_[b];
    if (rk_[a] == rk_[b]) ++rk_[a];
    return true;
  }

  /// 返回 x 所在连通块的大小
  int size_of(int x) { return sz_[find(x)]; }
};

// ================= 带权并查集 =================
/**
 * @brief 带权并查集：维护每个元素到根的权值差。
 *
 * 支持加权合并，可选模运算模式（值域 \f$[0, mod)\f$）。
 * 用法：unite(a, b, diff) 建立 \f$value(b) - value(a) = diff\f$ 的关系。
 */
class WeightedDSU {
  std::vector<int> fa_;
  std::vector<i64> wt_;    ///< wt_[x] = value(x) - value(fa_[x])
  bool mod_mode_;
  i64 mod_;

 public:
  /**
   * @brief 构造 n 个元素的带权并查集。
   * @param n 元素个数（编号 1..n）
   * @param mod true 为模运算模式（值域 [0, mod)），false 为整数值域
   * @param modulus 模数（仅 mod=true 时使用）
   */
  explicit WeightedDSU(int n, bool mod = false, i64 modulus = 0)
      : mod_mode_(mod), mod_(modulus) {
    fa_.resize(n + 1);
    wt_.assign(n + 1, 0);
    for (int i = 1; i <= n; ++i) fa_[i] = i;
  }

  /**
   * @brief 查询 x 所在集合的代表元（路径压缩时累加权值差）。
   * @param x 元素编号
   * @return x 所在集合的代表元
   */
  int find(int x) {
    if (fa_[x] == x) return x;
    int r = find(fa_[x]);
    wt_[x] += wt_[fa_[x]];          // 路径压缩累加权值
    fa_[x] = r;
    return r;
  }

  /**
   * @brief 返回 value(x)（以根为基准的相对值）。
   * @param x 元素编号
   * @return value(x)
   */
  i64 to_root(int x) { find(x); return wt_[x]; }

  /**
   * @brief 合并：建立 \f$value(b) - value(a) = diff\f$ 的关系。
   *
   * 若 a、b 已同集合，则校验既有约束是否一致。
   * @param a 元素编号
   * @param b 元素编号
   * @param diff 要求的权值差（b 相对 a）
   * @return 是否与已有约束一致（已同集合且矛盾时返回 false）
   */
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

  /**
   * @brief 判断 a、b 是否在同一集合。
   * @param a 元素编号
   * @param b 元素编号
   * @return 同一集合返回 true
   */
  bool same(int a, int b) { return find(a) == find(b); }

  /**
   * @brief 查询权值差 \f$value(b) - value(a)\f$（a、b 须同集合）。
   * @param a 元素编号
   * @param b 元素编号
   * @return value(b) - value(a)；模模式返回归一化结果
   */
  i64 diff(int a, int b) {          // value(b) - value(a)
    find(a); find(b);
    i64 d = to_root(b) - to_root(a);
    return mod_mode_ ? norm(d) : d;
  }

 private:
  /// 模意义下归约到 [0, mod)
  i64 norm(i64 x) const {
    x %= mod_;
    return x < 0 ? x + mod_ : x;
  }
};

// ================= 可撤销并查集 =================
/**
 * @brief 可撤销并查集：按大小合并（不做路径压缩），支持栈式回滚。
 *
 * 常配合线段树分治使用。
 */
class RollbackDSU {
  std::vector<int> fa_, sz_;
  struct Op { int u, v, szu; };     // 合并前记录
  std::vector<Op> hist_;

  /// 查询 x 所在集合的代表元（不路径压缩）
  int find(int x) const {
    while (fa_[x] != x) x = fa_[x];
    return x;
  }

 public:
  /**
   * @brief 构造 n 个元素的并查集。
   * @param n 元素个数（编号 1..n）
   */
  explicit RollbackDSU(int n) : fa_(n + 1), sz_(n + 1, 1) {
    for (int i = 1; i <= n; ++i) fa_[i] = i;
  }

  /**
   * @brief 合并 a、b 所在集合（按大小合并，记录回滚信息）。
   * @param a 元素编号
   * @param b 元素编号
   * @return 是否发生了实际合并（原本不在同一集合返回 true）
   */
  bool unite(int a, int b) {        // 按大小合并（不做路径压缩）
    int ra = find(a), rb = find(b);
    if (ra == rb) return false;
    if (sz_[ra] < sz_[rb]) std::swap(ra, rb);
    hist_.push_back({rb, ra, sz_[ra]});
    fa_[rb] = ra;
    sz_[ra] += sz_[rb];
    return true;
  }

  /**
   * @brief 判断 a、b 是否在同一集合。
   * @param a 元素编号
   * @param b 元素编号
   * @return 同一集合返回 true
   */
  bool same(int a, int b) const { return find(a) == find(b); }

  /**
   * @brief 回滚最近的一次 unite 操作。
   */
  void rollback() {
    WBWLIB_ASSERT(!hist_.empty());
    Op o = hist_.back(); hist_.pop_back();
    fa_[o.u] = o.u;
    sz_[o.v] = o.szu;
  }

  /// 返回当前操作历史大小，可作为回滚快照
  int snapshot() const { return (int)hist_.size(); }

  /**
   * @brief 回滚到快照 snap（撤销自 snap 之后的所有 unite）。
   * @param snap snapshot() 返回的快照值
   */
  void rollback_to(int snap) {
    while ((int)hist_.size() > snap) rollback();
  }
};

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_DSU_HPP