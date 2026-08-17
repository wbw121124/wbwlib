#ifndef WBWLIB_MATH_FLOOR_SUM_HPP
#define WBWLIB_MATH_FLOOR_SUM_HPP

/**
 * @file floor-sum.hpp
 * @brief 数论求和：ACL 风格 floor_sum（I/(n,m,a,b)：sum_{i=0}^{n-1} floor((a*i+b)/m)）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 * @par 复杂度
 * O(log (m + a))。
 *
 * 背景：朴素求和 O(n)，本函数用类欧几里得思想做到亚线性，常见于整除求和、
 * 逆序对/第 k 大等进阶题。
 *
 * @par 示例
 * @code{.cpp}
 * floor_sum(n, m, a, b)，所有参数非负。
 * @endcode
 */

#include <algorithm>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace math {

/**
 * @brief ACL 风格 floor_sum：计算 \f$\sum_{i=0}^{n-1} \left\lfloor \frac{a \cdot i + b}{m} \right\rfloor\f$。
 *
 * 用类欧几里得思想递归化归，复杂度 \f$O(\log(m + a))\f$。
 * 要求 n, m, a, b 均为非负整数。
 * @param n 项数
 * @param m 分母
 * @param a 一次项系数
 * @param b 常数项
 * @return \f$\sum_{i=0}^{n-1} \left\lfloor \frac{ai + b}{m} \right\rfloor\f$
 */
inline i64 floor_sum(i64 n, i64 m, i64 a, i64 b) {
  i64 ans = 0;
  for (;;) {
    if (a >= m) { ans += (n - 1) * n * (a / m) / 2; a %= m; }
    if (b >= m) { ans += n * (b / m); b %= m; }
    i64 y_max = a * n + b;
    if (y_max < m) break;
    n = y_max / m;
    b = y_max % m;
    std::swap(m, a);
  }
  return ans;
}

} // namespace math
} // namespace wbwlib

#endif // WBWLIB_MATH_FLOOR_SUM_HPP