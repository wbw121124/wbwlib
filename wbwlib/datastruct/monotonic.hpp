#ifndef WBWLIB_DS_MONOTONIC_HPP
#define WBWLIB_DS_MONOTONIC_HPP

/**
 * @file monotonic.hpp
 * @brief 单调栈、单调队列（滑动窗口最值）、最近小/大元素。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * 均为线性 O(n)。
 *
 * @par 示例
 * @code{.cpp}
 *   // 对每个位置求左侧最近的严格更小元素下标（1..n），无则 0
 *   auto lt = wbwlib::ds::prev_smaller(a);
 *   // 滑动窗口最小值（窗口长度 k，返回每个窗口的最小值序列）
 *   auto mn = wbwlib::ds::sliding_window_min(a, k);
 * @endcode
 */

#include <deque>
#include <functional>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace ds {

/**
 * @brief 对每个 i，求左侧最近的满足 cmp(a[j], a[i]) 的元素下标。
 *
 * 用单调栈维护；a 为 1 基（a[0] 占位），数据在 [1..n]。
 * @tparam T 元素类型
 * @tparam Cmp 比较函数类型（如 std::less 求左侧更小）
 * @param a 1 基数组
 * @param cmp 比较函数（满足即停止出栈）
 * @return res[i]：左侧最近满足条件的下标；不存在时为 0
 */
template<class T, class Cmp>
std::vector<int> prev_index(const std::vector<T>& a, Cmp cmp) {
  int n = (int)a.size() - 1;   // a[0] 占位，数据在 [1..n]
  std::vector<int> res(n + 1, 0);
  std::vector<int> st;
  for (int i = 1; i <= n; ++i) {
    while (!st.empty() && !cmp(a[st.back()], a[i])) st.pop_back();
    res[i] = st.empty() ? 0 : st.back();
    st.push_back(i);
  }
  return res;
}

/**
 * @brief 对每个 i，求右侧最近的满足 cmp(a[i], a[j]) 的元素下标。
 *
 * 用单调栈维护；a 为 1 基（a[0] 占位），数据在 [1..n]。
 * @tparam T 元素类型
 * @tparam Cmp 比较函数类型
 * @param a 1 基数组
 * @param cmp 比较函数（满足即停止出栈）
 * @return res[i]：右侧最近满足条件的下标；不存在时为 n+1
 */
template<class T, class Cmp>
std::vector<int> next_index(const std::vector<T>& a, Cmp cmp) {
  int n = (int)a.size() - 1;   // a[0] 占位，数据在 [1..n]
  std::vector<int> res(n + 2, n + 1);
  std::vector<int> st;
  for (int i = n; i >= 1; --i) {
    while (!st.empty() && !cmp(a[i], a[st.back()])) st.pop_back();
    res[i] = st.empty() ? n + 1 : st.back();
    st.push_back(i);
  }
  return res;
}

/**
 * @brief 每个位置左侧最近的严格更小元素下标。
 * @param a 1 基数组（a[0] 占位）
 * @return res[i]：左侧最近严格更小的下标；不存在时为 0
 */
inline std::vector<int> prev_smaller(const std::vector<i64>& a) {
  return prev_index(a, [](i64 x, i64 y) { return x < y; });
}

/**
 * @brief 每个位置右侧最近的严格更小元素下标。
 * @param a 1 基数组（a[0] 占位）
 * @return res[i]：右侧最近严格更小的下标；不存在时为 n+1
 */
inline std::vector<int> next_smaller(const std::vector<i64>& a) {
  return next_index(a, [](i64 x, i64 y) { return x > y; });
}

/**
 * @brief 每个位置左侧最近的严格更大元素下标。
 * @param a 1 基数组（a[0] 占位）
 * @return res[i]：左侧最近严格更大的下标；不存在时为 0
 */
inline std::vector<int> prev_greater(const std::vector<i64>& a) {
  return prev_index(a, [](i64 x, i64 y) { return x > y; });
}

/**
 * @brief 每个位置右侧最近的严格更大元素下标。
 * @param a 1 基数组（a[0] 占位）
 * @return res[i]：右侧最近严格更大的下标；不存在时为 n+1
 */
inline std::vector<int> next_greater(const std::vector<i64>& a) {
  return next_index(a, [](i64 x, i64 y) { return x < y; });
}

/**
 * @brief 滑动窗口最值：对每个长度为 k 的窗口求最值。
 *
 * 用双端队列维护单调序列；a 为 1 基（a[0] 占位），数据在 [1..n]。
 * @tparam T 元素类型
 * @tparam Cmp 比较函数类型（better(x, y) 表示 x 比 y 更优）
 * @param a 1 基数组
 * @param k 窗口大小
 * @param better 最值比较（如 std::less 求最小值）
 * @return 每个窗口的最值（共 n-k+1 个，按窗口起点 1..n-k+1 排列）
 */
template<class T, class Cmp>
std::vector<T> sliding_window(const std::vector<T>& a, int k, Cmp better) {
  int n = (int)a.size() - 1;   // a[0] 占位，数据在 [1..n]
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

/**
 * @brief 滑动窗口最小值。
 * @param a 1 基数组（a[0] 占位）
 * @param k 窗口大小
 * @return 每个窗口的最小值（共 n-k+1 个）
 */
inline std::vector<i64> sliding_window_min(const std::vector<i64>& a, int k) {
  return sliding_window(a, k, [](i64 x, i64 y) { return x < y; });
}

/**
 * @brief 滑动窗口最大值。
 * @param a 1 基数组（a[0] 占位）
 * @param k 窗口大小
 * @return 每个窗口的最大值（共 n-k+1 个）
 */
inline std::vector<i64> sliding_window_max(const std::vector<i64>& a, int k) {
  return sliding_window(a, k, [](i64 x, i64 y) { return x > y; });
}

} // namespace ds
} // namespace wbwlib

#endif // WBWLIB_DS_MONOTONIC_HPP