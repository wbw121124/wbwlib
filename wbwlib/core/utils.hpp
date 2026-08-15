#ifndef WBWLIB_CORE_UTILS_HPP
#define WBWLIB_CORE_UTILS_HPP

/**
 * @file utils.hpp
 * @brief 通用小工具：多变元 min/max、clamp、离散化（坐标压缩）、计时器。
 *
 * 依赖：core/base.hpp
 */

#include <algorithm>
#include <chrono>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace core {

// ---------- 多变元 min / max ----------
template<class T>
inline T wmin(const T& a) { return a; }                  // 单参数兜底

template<class T, class U, class... Rest>
inline auto wmin(const T& a, const U& b, const Rest&... rest)
    -> decltype(std::min(a, b)) {                        // 语法保证返回类型一致
  return wmin(std::min(a, b), rest...);
}

template<class T>
inline T wmax(const T& a) { return a; }

template<class T, class U, class... Rest>
inline auto wmax(const T& a, const U& b, const Rest&... rest)
    -> decltype(std::max(a, b)) {
  return wmax(std::max(a, b), rest...);
}

/// 将 v 夹到 [lo, hi]（v < lo 返回 lo，v > hi 返回 hi）
template<class T>
inline const T& clamp(const T& v, const T& lo, const T& hi) {
  return (v < lo) ? lo : ((hi < v) ? hi : v);
}

// ---------- 离散化（坐标压缩） ----------
/**
 * 将序列 a 离散化，返回每个 a[i] 对应到的 0 基排名。
 * std::unique 支持自定义去重方式（额外可传入比较器？此处提供两种重载）。
 * 原地修改传入的 rank 结果。
 *
 * 用法：
 *   vector<long long> a = {3, 1, 4, 1};
 *   vector<long long>& r = discretize(a);   // 返回引用，r 为 {2,0,1,0}
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

/// 返回 (rank, sorted_unique)，避免重复排序
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
class Stopwatch {
  using clk = std::chrono::steady_clock;
  clk::time_point t0_;
 public:
  Stopwatch() { reset(); }
  void reset() { t0_ = clk::now(); }
  /// 返回自上次 reset 以来的毫秒数
  double ms() const {
    return std::chrono::duration<double, std::milli>(clk::now() - t0_).count();
  }
};

} // namespace core
} // namespace wbwlib

#endif // WBWLIB_CORE_UTILS_HPP