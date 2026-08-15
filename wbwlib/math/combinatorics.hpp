#ifndef WBWLIB_MATH_COMBINATORICS_HPP
#define WBWLIB_MATH_COMBINATORICS_HPP

/**
 * @file combinatorics.hpp
 * @brief 组合计数工具：阶乘/逆元预计算、组合数、排列数、Lucas、卡特兰、
 *        两型斯特林数、错排数。
 *
 * 依赖：wbwlib/core/base.hpp, wbwlib/math/modular.hpp
 *
 * 通用类型参数 M 为模整数类（wbwlib::math::modint<MOD> 或 modint_dyn）。
 */

#include "wbwlib/core/base.hpp"
#include "wbwlib/math/modular.hpp"

namespace wbwlib {
namespace math {

/**
 * 组合数学预计算结构体。
 *
 * 用法：
 *   using M = modint<1'000'000'007>;
 *   Comb<M> cb(100000);     // 预计算 0..100000 的阶乘
 *   M v = cb.C(10, 3);      // 组合数 C(10,3)=120
 *   M p = cb.P(10, 3);      // 排列数 10*9*8
 *
 * 复杂度：构造 O(n)，单次查询 O(1)。
 */
template<class M>
struct Comb {
  std::vector<M> fact, ifact;   ///< 阶乘与阶乘逆元
  int n_;

  explicit Comb(int n = 0) : n_(n) {
    fact.resize(n + 1);
    ifact.resize(n + 1);
    fact[0] = M(1);
    for (int i = 1; i <= n; ++i) fact[i] = fact[i - 1] * M(i);
    ifact[n] = M(1) / fact[n];   // 费马/扩展欧几里得一次
    for (int i = n; i >= 1; --i) ifact[i - 1] = ifact[i] * M(i);
  }

  M C(int n, int k) const {       ///< C(n,k)，越界返回 0
    if (k < 0 || k > n) return M(0);
    return fact[n] * ifact[k] * ifact[n - k];
  }

  M P(int n, int k) const {       ///< 排列数 A(n,k)
    if (k < 0 || k > n) return M(0);
    return fact[n] * ifact[n - k];
  }

  M inv_of(int x) const { return ifact[x] * fact[x - 1]; }  ///< x 的逆元（x<=n）

  int size() const { return n_; }
};

// ================= Lucas 定理 =================
/**
 * Lucas(n, k) mod p，p 为质数，用于 n, k 超过预计算范围（按 p 进制分治）。
 * 需要 Comb 已预计算阶乘（至少到 p-1）。
 *
 * C(n, k) ≡ C(n%p, k%p) * C(n/p, k/p) (mod p)，O(p + log_p(n))。
 */
template<class M>
inline M lucas_comb(i64 n, i64 k, i64 p, const Comb<M>& cb) {
  if (k < 0 || k > n) return M(0);
  if (k == 0) return M(1);
  return cb.C((int)(n % p), (int)(k % p)) * lucas_comb(n / p, k / p, p, cb);
}

// ================= 卡特兰数 =================
/// Catalan(n) = C(2n, n) / (n+1)
template<class M>
inline M catalan(int n, const Comb<M>& cb) {
  return cb.C(2 * n, n) / M(n + 1);
}

// ================= 第二类斯特林数 =================
/**
 * n 个球放入 k 个非空集合的方案数。
 * 递推：S(n,k)=S(n-1,k-1)+k*S(n-1,k)。提供单点计算版（O(nk) 空间 O(k)）。
 */
template<class M>
inline M stirling2(int n, int k) {
  if (k < 0 || k > n) return M(0);
  std::vector<M> dp(k + 1, M(0));
  dp[0] = M(1);
  for (int i = 1; i <= n; ++i)
    for (int j = (std::min)(i, k); j >= 1; --j)
      dp[j] = dp[j - 1] + dp[j] * M(j);
  return dp[k];
}

// ================= 第一类斯特林数 =================
/**
 * S1(n,k)：n 个元素排列成 k 个环的方案数。
 * 递推：S1(n,k)=S1(n-1,k-1)+(n-1)*S1(n-1,k)。
 */
template<class M>
inline M stirling1(int n, int k) {
  if (k < 0 || k > n) return M(0);
  std::vector<M> dp(k + 1, M(0));
  dp[0] = M(1);
  for (int i = 1; i <= n; ++i)
    for (int j = (std::min)(i, k); j >= 1; --j)
      dp[j] = dp[j - 1] + dp[j] * M(i - 1);
  return dp[k];
}

// ================= 错排数 =================
/// D(n) = (n-1)*(D(n-1)+D(n-2))，D(0)=1, D(1)=0
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