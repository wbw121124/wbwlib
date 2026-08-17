#ifndef WBWLIB_STR_CHAR_MAP_HPP
#define WBWLIB_STR_CHAR_MAP_HPP

/**
 * @file char-map.hpp
 * @brief 字符 → 字母表下标映射策略（供 Trie / AC 自动机 / 后缀自动机 / 回文自动机模板化使用）。
 *
 * @par 复杂度
 * O(1)。
 *
 * @par 示例
 * @code{.cpp}
 *   // 默认小写字母映射：[a,z] -> [0,26)
 *   LowerCharMap::id('b');            // 1
 *   // 自定义：数字串
 *   struct DigitMap { static int id(char c){ return c-'0'; } static bool valid(char c){ return c>='0'&&c<='9'; } };
 * @endcode
 */

namespace wbwlib {
namespace str {

/**
 * @brief 默认字符映射：小写字母 → [0, 26)。
 */
struct LowerCharMap {
  /// 字符转下标（假定已通过 valid 校验）
  static int id(char c) { return c - 'a'; }
  /// 判断字符是否在字母表内
  static bool valid(char c) { return c >= 'a' && c <= 'z'; }
};

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_CHAR_MAP_HPP
