#ifndef WBWLIB_DS_MONOTONIC_HPP
#define WBWLIB_DS_MONOTONIC_HPP

/**
 * @file monotonic.hpp
 * @brief 单调栈、单调队列（滑动窗口最值）、最近小/大元素。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：均为线性 O(n)。
 *
 * 用法：
 *   // 对每个位置求左侧最近的严格更小元素下标（1..n），无则 0
 *   auto lt = wbwlib::ds::prev_smaller(a);
 *   // 滑动窗口最小值（窗口长度 k，返回每个窗口的最小值序列）
 *   auto mn = wbwlib::ds::sliding_window_min(a, k);
 */

#include <deque>
#include <functional>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

/// 对每个 i，求左侧最近的满足 cmp(a[j], a[i]) 的元素下标；j < i，无则 0
template<class T, class Cmp>
std::vector<int> prev_index(const std::vector<T>& a, Cmp cmp) {
  int n = (int)a.size();
  std::vector<int> res(n + 1, 0);
  std::vector<int> st;
  for (int i = 1; i <= n; ++i) {
    while (!st.empty() && !cmp(a[st.back()], a[i])) st.pop_back();
    res[i] = st.empty() ? 0 : st.back();
    st.push_back(i);
  }
  return res;
}

/// 对每个 i，求右侧最近的满足 cmp(a[i], a[j]) 的元素下标；j > i，无则 n+1
template<class T, class Cmp>
std::vector<int> next_index(const std::vector<T>& a, Cmp cmp) {
  int n = (int)a.size();
  std::vector<int> res(n + 2, n + 1);
  std::vector<int> st;
  for (int i = n; i >= 1; --i) {
    while (!st.empty() && !cmp(a[i], a[st.back()])) st.pop_back();
    res[i] = st.empty() ? n + 1 : st.back();
    st.push_back(i);
  }
  return res;
}

/// 左侧最近的严格更小（返回下标，a 为 1 基，a[0] 占位）
inline std::vector<int> prev_smaller(const std::vector<i64>& a) {
  return prev_index(a, [](i64 x, i64 y) { return x < y; });
}
/// 右侧最近的严格更小
inline std::vector<int> next_smaller(const std::vector<i64>& a) {
  return next_index(a, [](i64 x, i64 y) { return x < y; });
}
/// 左侧最近的严格更大
inline std::vector<int> prev_greater(const std::vector<i64>& a) {
  return prev_index(a, [](i64 x, i64 y) { return x > y; });
}
/// 右侧最近的严格更大
inline std::vector<int> next_greater(const std::vector<i64>& a) {
  return next_index(a, [](i64 x, i64 y) { return x > y; });
}

/// 滑动窗口最值：窗口大小 k，返回每个窗口的最值（共 n-k+1 个）
template<class T, class Cmp>
std::vector<T> sliding_window(const std::vector<T>& a, int k, Cmp better) {
  int n = (int)a.size();
  std::vector<T> res;
  res.reserve(n - k + 1);
  std::deque<int> dq;
  for (int i = 1; i <= n; ++i) {
    while (!dq.empty() && better(a[i], a[dq.back()])) dq.pop_back();
    dq.push_back(i);
    if (dq.front() <= i - k) dq.pop_front();
    if (i >= k) res.push_back(a[dq.front()]);
  }
  return res;
}

inline std::vector<i64> sliding_window_min(const std::vector<i64>& a, int k) {
  return sliding_window(a, k, [](i64 x, i64 y) { return x < y; });
}
inline std::vector<i64> sliding_window_max(const std::vector<i64>& a, int k) {
  return sliding_window(a, k, [](i64 x, i64 y) { return x > y; });
}

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_MONOTONIC_HPP