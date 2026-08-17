#ifndef WBWLIB_CORE_UTILS_HPP
#define WBWLIB_CORE_UTILS_HPP

/**
 * @file utils.hpp
 * @brief 通用小工具：多变元 min/max、clamp、离散化（坐标压缩）、计时器。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 */

#include <algorithm>
#include <chrono>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace core {

// ---------- 多变元 min / max ----------
/**
 * @brief 单参数兜底版本，直接返回自身。
 * @param a 待返回的值
 * @return a
 */
template<class T>
inline T wmin(const T& a) { return a; }                  // 单参数兜底

/**
 * @brief 多变元最小值：返回 a, b, rest... 中的最小值。
 * @param a 第一个值
 * @param b 第二个值
 * @param rest 其余值（可为空）
 * @return 全部参数中的最小值
 */
template<class T, class U, class... Rest>
inline auto wmin(const T& a, const U& b, const Rest&... rest)
    -> typename std::decay<decltype(std::min(a, b))>::type {
  return wmin(std::min(a, b), rest...);
}

/**
 * @brief 单参数兜底版本，直接返回自身。
 * @param a 待返回的值
 * @return a
 */
template<class T>
inline T wmax(const T& a) { return a; }

/**
 * @brief 多变元最大值：返回 a, b, rest... 中的最大值。
 * @param a 第一个值
 * @param b 第二个值
 * @param rest 其余值（可为空）
 * @return 全部参数中的最大值
 */
template<class T, class U, class... Rest>
inline auto wmax(const T& a, const U& b, const Rest&... rest)
    -> typename std::decay<decltype(std::max(a, b))>::type {
  return wmax(std::max(a, b), rest...);
}

/**
 * @brief 将 v 夹到 [lo, hi] 区间内。
 * @param v 待夹取的值
 * @param lo 区间下界
 * @param hi 区间上界
 * @return \f$\min(\max(v, lo), hi)\f$
 */
template<class T>
inline const T& clamp(const T& v, const T& lo, const T& hi) {
  return (v < lo) ? lo : ((hi < v) ? hi : v);
}

// ---------- 离散化（坐标压缩） ----------
/**
 * @brief 将序列 a 离散化（坐标压缩），返回每个 a[i] 对应的 0 基排名。
 *
 * 排名公式：\f$r_i = |\{j : x_j < a_i\}|\f$，其中 \f$x\f$ 为 a 去重排序后的序列。
 *
 * @tparam T 元素类型（需支持 < 比较）
 * @param a 原序列
 * @return 与 a 等长的排名序列（值域为 0..去重后个数-1）
 */
template<class T>
inline std::vector<T> discretize(const std::vector<T>& a) {
  std::vector<T> xs(a);
  std::sort(xs.begin(), xs.end());
  xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
  std::vector<T> r(a.size());
  for (size_t i = 0; i < a.size(); ++i)
    r[i] = static_cast<T>(std::lower_bound(xs.begin(), xs.end(), a[i]) - xs.begin());
  return r;
}

/**
 * @brief 离散化并同时返回去重排序后的序列，避免重复排序。
 * @tparam T 元素类型（需支持 < 比较）
 * @param a 原序列
 * @return pair：first 为排名序列，second 为去重排序后的唯一值序列
 */
template<class T>
std::pair<std::vector<T>, std::vector<T>> discretize_full(const std::vector<T>& a) {
  std::vector<T> xs(a);
  std::sort(xs.begin(), xs.end());
  xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
  std::vector<T> r(a.size());
  for (size_t i = 0; i < a.size(); ++i)
    r[i] = static_cast<T>(std::lower_bound(xs.begin(), xs.end(), a[i]) - xs.begin());
  return {std::move(r), std::move(xs)};
}

// ---------- 计时器（本地调试用） ----------
/**
 * @brief 计时器（本地调试用）：基于 steady_clock 测量耗时。
 */
class Stopwatch {
  using clk = std::chrono::steady_clock;
  clk::time_point t0_;
 public:
  /**
   * @brief 构造并开始计时。
   */
  Stopwatch() { reset(); }

  /**
   * @brief 重置计时起点为当前时刻。
   */
  void reset() { t0_ = clk::now(); }

  /**
   * @brief 返回自上次 reset 以来的毫秒数。
   * @return 已耗时（毫秒，double）
   */
  double ms() const {
    return std::chrono::duration<double, std::milli>(clk::now() - t0_).count();
  }
};

} // namespace core
} // namespace wbwlib

#endif // WBWLIB_CORE_UTILS_HPP