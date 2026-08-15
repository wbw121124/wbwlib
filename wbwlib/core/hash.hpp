#ifndef WBWLIB_CORE_HASH_HPP
#define WBWLIB_CORE_HASH_HPP

/**
 * @file hash.hpp
 * @brief 抗哈希冲突的哈希器（用于 std::unordered_map / unordered_set）。
 *
 * 依赖：core/base.hpp, core/random.hpp
 *
 * 背景：标准库 std::hash 对整数的实现为恒等映射，攻击者可构造大量哈希冲突
 * 导致 O(n^2) 退化。本文件用 splitmix64 打乱低位位模式，抗对抗构造。
 *
 * 用法：
 *   unordered_map<K,V, wbwlib::core::splitmix_hash> mp;
 *   unordered_set<P, wbwlib::core::pair_hash> st;   // P 为 pair<...>
 */

#include <cstddef>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "wbwlib/core/base.hpp"
#include "wbwlib/core/random.hpp"

namespace wbwlib {
namespace core {

/// 通用整数/可哈希对象的 splitmix64 哈希器
struct splitmix_hash {
  template<class T>
  std::size_t operator()(const T& v) const {
    // 固定随机偏移，使碰撞模式与输入无关
    static const u64 FIXED_RANDOM =
        std::chrono::steady_clock::now().time_since_epoch().count();
    u64 h = splitmix64(static_cast<u64>(std::hash<T>{}(v)));
    return static_cast<std::size_t>(h ^ splitmix64(FIXED_RANDOM));
  }
};

/// pair 哈希：两级 splitmix64 混合（用于二维哈希表键）
struct pair_hash {
  template<class A, class B>
  std::size_t operator()(const std::pair<A, B>& p) const {
    u64 h1 = splitmix64(static_cast<u64>(std::hash<A>{}(p.first)) + 0x9e3779b97f4a7c15ULL);
    u64 h2 = static_cast<u64>(std::hash<B>{}(p.second));
    u64 h = h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
    return static_cast<std::size_t>(splitmix64(h));
  }
};

// 便捷别名
template<class K, class V, class H = splitmix_hash>
using umap = std::unordered_map<K, V, H>;

template<class K, class H = splitmix_hash>
using uset = std::unordered_set<K, H>;

} // namespace core
} // namespace wbwlib

#endif // WBWLIB_CORE_HASH_HPP