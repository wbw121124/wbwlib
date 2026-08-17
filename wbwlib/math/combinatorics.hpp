#ifndef WBWLIB_MATH_COMBINATORICS_HPP
#define WBWLIB_MATH_COMBINATORICS_HPP

/**
 * @file combinatorics.hpp
 * @brief 组合计数工具：阶乘/逆元预计算、组合数、排列数、Lucas、卡特兰、
 *        两型斯特林数、错排数。
 *
 * @par 依赖
 * wbwlib/core/base.hpp, wbwlib/math/modular.hpp
 *
 * 通用类型参数 M 为模整数类（wbwlib::math::modint<MOD> 或 modint_dyn）。
 */

#include "wbwlib/core/base.hpp"
#include "wbwlib/math/modular.hpp"

namespace wbwlib {
namespace math {

/**
 * @brief 组合数学预计算结构体：阶乘与阶乘逆元，O(1) 查询组合数/排列数。
 *
 * 用法：
 *   using M = modint<1'000'000'007>;
 *   Comb<M> cb(100000);     // 预计算 0..100000 的阶乘
 *   M v = cb.C(10, 3);      // 组合数 C(10,3)=120
 *   M p = cb.P(10, 3);      // 排列数 10*9*8
 *
 * 复杂度：构造 O(n)，单次查询 O(1)。
 * @tparam M 模整数类（wbwlib::math::modint<MOD> 或 modint_dyn）
 */
template<class M>
struct Comb {
  std::vector<M> fact, ifact;   ///< 阶乘与阶乘逆元
  int n_;

  /**
   * @brief 构造并预计算 0..n 的阶乘与阶乘逆元。
   * @param n 最大下标
   */
  explicit Comb(int n = 0) : n_(n) {
    fact.resize(n + 1);
    ifact.resize(n + 1);
    fact[0] = M(1);
    for (int i = 1; i <= n; ++i) fact[i] = fact[i - 1] * M(i);
    ifact[n] = M(1) / fact[n];   // 费马/扩展欧几里得一次
    for (int i = n; i >= 1; --i) ifact[i - 1] = ifact[i] * M(i);
  }

  /**
   * @brief 组合数 \f$\binom{n}{k}\f$。
   * @param n 总数
   * @param k 选取数
   * @return \f$\binom{n}{k} = \frac{n!}{k!(n-k)!}\f$；越界返回 0
   */
  M C(int n, int k) const {
    if (k < 0 || k > n) return M(0);
    return fact[n] * ifact[k] * ifact[n - k];
  }

  /**
   * @brief 排列数 \f$A(n, k) = \frac{n!}{(n-k)!}\f$。
   * @param n 总数
   * @param k 选取数
   * @return 排列数；越界返回 0
   */
  M P(int n, int k) const {
    if (k < 0 || k > n) return M(0);
    return fact[n] * ifact[n - k];
  }

  /**
   * @brief x 的逆元（要求 x <= n 且 x > 0）。
   * @param x 求逆的数
   * @return \f$x^{-1}\f$
   */
  M inv_of(int x) const { return ifact[x] * fact[x - 1]; }

  /// 返回预计算规模 n
  int size() const { return n_; }
};

// ================= Lucas 定理 =================
/**
 * @brief Lucas 定理求组合数 \f$\binom{n}{k} \bmod p\f$，p 为质数，用于 n, k 超过预计算范围。
 *
 * 递归式：\f$\binom{n}{k} \equiv \binom{n \bmod p}{k \bmod p} \cdot \binom{\lfloor n/p \rfloor}{\lfloor k/p \rfloor} \pmod p\f$，
 * 复杂度 \f$O(p + \log_p n)\f$。需要 Comb 已预计算阶乘（至少到 p-1）。
 *
 * @tparam M 模整数类
 * @param n 总数（可大于预计算范围）
 * @param k 选取数
 * @param p 质数模数
 * @param cb 已预计算的组合数结构（阶乘至少覆盖 0..p-1）
 * @return \f$\binom{n}{k} \bmod p\f$；越界返回 0
 */
template<class M>
inline M lucas_comb(i64 n, i64 k, i64 p, const Comb<M>& cb) {
  if (k < 0 || k > n) return M(0);
  if (k == 0) return M(1);
  return cb.C((int)(n % p), (int)(k % p)) * lucas_comb(n / p, k / p, p, cb);
}

/**
 * @brief 卡特兰数：\f$Cat(n) = \frac{1}{n+1}\binom{2n}{n}\f$。
 * @tparam M 模整数类
 * @param n 下标（非负）
 * @param cb 已预计算的组合数结构（需覆盖 2n）
 * @return \f$Cat(n)\f$
 */
template<class M>
inline M catalan(int n, const Comb<M>& cb) {
  return cb.C(2 * n, n) / M(n + 1);
}

// ================= 第二类斯特林数 =================
/**
 * @brief 第二类斯特林数 \f$S(n, k)\f$：把 n 个有区别的球放入 k 个非空集合的方案数。
 *
 * 递推：\f$S(n, k) = S(n-1, k-1) + k \cdot S(n-1, k)\f$。时间复杂度 \f$O(nk)\f$，空间 \f$O(k)\f$。
 * @tparam M 模整数类
 * @param n 球数
 * @param k 集合数
 * @return \f$S(n, k)\f$；越界返回 0
 */
template<class M>
inline M stirling2(int n, int k) {
  if (k < 0 || k > n) return M(0);
  std::vector<M> dp(k + 1, M(0));
  dp[0] = M(1);
  for (int i = 1; i <= n; ++i) {
    for (int j = (std::min)(i, k); j >= 1; --j)
      dp[j] = dp[j - 1] + dp[j] * M(j);
    dp[0] = M(0);                 // 下一行 j=1 需要 S(i,0)=0（仅 i=0 行为 1）
  }
  return dp[k];
}

// ================= 第一类斯特林数 =================
/**
 * @brief 第一类斯特林数 \f$S_1(n, k)\f$：把 n 个元素排列成 k 个环的方案数。
 *
 * 递推：\f$S_1(n, k) = S_1(n-1, k-1) + (n-1) \cdot S_1(n-1, k)\f$。
 * 时间复杂度 \f$O(nk)\f$，空间 \f$O(k)\f$。
 * @tparam M 模整数类
 * @param n 元素数
 * @param k 环数
 * @return \f$S_1(n, k)\f$；越界返回 0
 */
template<class M>
inline M stirling1(int n, int k) {
  if (k < 0 || k > n) return M(0);
  std::vector<M> dp(k + 1, M(0));
  dp[0] = M(1);
  for (int i = 1; i <= n; ++i) {
    for (int j = (std::min)(i, k); j >= 1; --j)
      dp[j] = dp[j - 1] + dp[j] * M(i - 1);
    dp[0] = M(0);                 // 下一行 j=1 需要 S1(i,0)=0
  }
  return dp[k];
}

// ================= 错排数 =================
/**
 * @brief 错排数：n 个元素全排列中没有任何一个元素留在原位的方案数。
 *
 * 递推：\f$D(n) = (n-1)(D(n-1) + D(n-2))\f$，边界 \f$D(0) = 1, D(1) = 0\f$。
 * @tparam M 模整数类
 * @param n 元素数（非负）
 * @return \f$D(n)\f$
 */
template<class M>
inline M derangement(int n) {
  M d0 = M(1), d1 = M(0);
  if (n == 0) return d0;
  if (n == 1) return d1;
  for (int i = 2; i <= n; ++i) {
    M d2 = M(i - 1) * (d0 + d1);
    d0 = d1; d1 = d2;
  }
  return d1;
}

} // namespace math
} // namespace wbwlib

#endif // WBWLIB_MATH_COMBINATORICS_HPP