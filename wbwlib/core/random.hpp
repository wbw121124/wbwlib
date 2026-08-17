#ifndef WBWLIB_CORE_RANDOM_HPP
#define WBWLIB_CORE_RANDOM_HPP

/**
 * @file random.hpp
 * @brief 随机工具：splitmix64、mt19937_64 封装、随机排列。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * splitmix64 用于：
 *  1) 生成稳定的伪随机种子；
 *  2) hash.hpp 中作为抗哈希冲突的散列函数（规避无序容器的对抗性构造）。
 */

#include <algorithm>
#include <chrono>
#include <iterator>
#include <random>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace core {

/**
 * @brief 64 位 splitmix64 伪随机函数：快速且分布均匀，常用于生成种子或做哈希散列。
 * @param x 输入值（可为任意 64 位整数）
 * @return 混合后的 64 位伪随机值
 */
inline u64 splitmix64(u64 x) {
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}

/**
 * @brief MT19937_64 无缝封装（线程不安全，OI 单线程足够）。
 */
struct Rand {
  std::mt19937_64 mt;

  /**
   * @brief 构造：以系统时钟+地址偏移生成的种子初始化 mt19937_64。
   */
  Rand() : mt(Rand::seed()) {}

  /**
   * @brief 用系统时钟（纳秒）+ 地址偏移构造种子，保证每次运行不同。
   * @return 64 位种子值
   */
  static u64 seed() {
    u64 nano = static_cast<u64>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    return splitmix64(nano ^ (u64)(std::uintptr_t)&nano);
  }

  /**
   * @brief 用指定种子构造。
   * @param s 种子值
   */
  explicit Rand(u64 s) : mt(s) {}

  /**
   * @brief 生成 [0, 2^64) 均匀分布的随机数。
   * @return 64 位无符号随机数
   */
  u64 next() { return mt(); }

  /**
   * @brief 生成 [l, r] 闭区间内的随机数。
   * @param l 区间左端点
   * @param r 区间右端点
   * @return 区间内均匀的随机整数
   */
  u64 range(u64 l, u64 r) {
    return l + (mt() % (r - l + 1));
  }

  /**
   * @brief 生成 [0, n) 内的随机数。
   * @param n 上界（不含）
   * @return 模 n 的均匀随机整数
   */
  u64 below(u64 n) { return mt() % n; }

  /**
   * @brief 0/1 掷硬币。
   * @return 以近似等概率返回 true/false
   */
  bool coin() { return (mt() & 1) != 0; }

  /**
   * @brief 打乱容器（传入迭代器对）。
   * @param first 起始迭代器
   * @param last 末尾迭代器
   */
  template<class Iter>
  void shuffle(Iter first, Iter last) {
    for (auto it = last - first; it > 1; --it) {
      std::iter_swap(first + (it - 1),
                     first + static_cast<long>(mt() % static_cast<u64>(it)));
    }
  }

  /**
   * @brief 打乱容器的引用重载（与迭代器对版本互不冲突）。
   * @param c 待打乱的容器
   */
  template<class Container>
  void shuffle(Container& c) {
    if (c.size() > 1)
      shuffle(std::begin(c), std::end(c));
  }
};

// C++14 无 inline 变量，使用每翻译单元一份的 static 实例（OI 单编译单元足够）
static Rand rng;  ///< 全局随机数发生器（在本 TU 内访问）

} // namespace core
} // namespace wbwlib

#endif // WBWLIB_CORE_RANDOM_HPP