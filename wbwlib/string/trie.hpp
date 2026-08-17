#ifndef WBWLIB_STR_TRIE_HPP
#define WBWLIB_STR_TRIE_HPP

/**
 * @file trie.hpp
 * @brief 字典树（Trie）：模板化字符映射与字母表大小；01-Trie 模板化位宽与值类型。
 *
 * @par 依赖
 * wbwlib/core/base.hpp, wbwlib/string/char-map.hpp
 *
 * @par 复杂度
 * 插入/查询 O(len) 或 O(位数)。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::str::Trie<> tr;                 // 默认：小写字母
 *   tr.insert("abc"); tr.insert("abd");
 *   tr.count_prefix("ab");                  // 以 "ab" 为前缀的单词数
 *   tr.search("abd")==1;                    // 是否存在
 *
 *   // 自定义：数字串（字符映射 + 字母表大小）
 *   struct DigitMap { static int id(char c){ return c-'0'; } static bool valid(char c){ return c>='0'&&c<='9'; } };
 *   wbwlib::str::Trie<char, 10, DigitMap> dt;
 *   dt.insert("123"); dt.insert("456");
 *
 *   // 01-Trie：xor_max(x) 返回与 x 异或最大值对应元素的异或值
 *   wbwlib::str::Trie01<> t;                // 默认 u32/32 位
 *   t.insert(5); int best = t.xor_max(8);
 *   wbwlib::str::Trie01<unsigned long long, 64> t64;
 * @endcode
 */

#include <string>
#include <vector>
#include "wbwlib/core/base.hpp"
#include "wbwlib/string/char-map.hpp"

namespace wbwlib {
namespace str {

// ================= 字符串 Trie =================
/**
 * @brief 模板化字典树：字符类型、字母表大小、字符映射均可自定义。
 * @tparam Char    字符类型（默认 char）
 * @tparam ALPHA   字母表大小（默认 26）
 * @tparam CharMap 字符映射策略：静态 id(char)->[0,ALPHA)，静态 valid(char)->bool（默认小写字母）
 */
template<class Char = char, int ALPHA = 26, class CharMap = LowerCharMap>
class Trie {
  struct Node { int nxt[ALPHA]; int cnt, end; };
  std::vector<Node> tr_;

 public:
  /// 构造空 Trie，自动创建根节点 tr_[0]（Node 花括号 value-init 清零）
  Trie() { tr_.push_back({}); }

  /**
   * @brief 插入字符串 s。
   * @param s 待插入字符串（字符集须被 CharMap::valid 覆盖）
   */
  void insert(const std::basic_string<Char>& s) {
    int u = 0;
    for (Char c : s) {
      int id = CharMap::id(c);
      if (tr_[u].nxt[id] == 0) {
        tr_[u].nxt[id] = (int)tr_.size();
        tr_.push_back(Node{});
      }
      u = tr_[u].nxt[id];
      ++tr_[u].cnt;
    }
    ++tr_[u].end;
  }

  /**
   * @brief 查询 s 作为完整单词被插入的次数。
   * @param s 待查询字符串
   * @return 出现次数；s 不在 Trie 中为 0
   */
  int count(const std::basic_string<Char>& s) const {
    int u = 0;
    for (Char c : s) {
      int id = CharMap::id(c);
      if (tr_[u].nxt[id] == 0) return 0;
      u = tr_[u].nxt[id];
    }
    return tr_[u].end;
  }

  /**
   * @brief 查询以 s 为前缀的字符串数量。
   * @param s 待查询前缀
   * @return 以 s 为前缀的单词数；s 不在 Trie 中为 0
   */
  int count_prefix(const std::basic_string<Char>& s) const {
    int u = 0;
    for (Char c : s) {
      int id = CharMap::id(c);
      if (tr_[u].nxt[id] == 0) return 0;
      u = tr_[u].nxt[id];
    }
    return tr_[u].cnt;
  }

  /**
   * @brief 判断 s 是否为已插入的完整单词。
   * @param s 待查询字符串
   * @return 存在则返回 true
   */
  bool search(const std::basic_string<Char>& s) const { return count(s) > 0; }
};

// ================= 01-Trie（异或查询）=================
/**
 * @brief 模板化 01-Trie：值类型与位宽可自定义，支持插入与异或最大值查询。
 * @tparam T    值类型（须为无符号整型，默认 u32）
 * @tparam BITS 位宽（默认 32；64 位可传 64）
 */
template<class T = u32, int BITS = 32>
class Trie01 {
  struct Node { int nxt[2]; int cnt; };
  std::vector<Node> tr_;

 public:
  /// 构造空 01-Trie，自动创建根节点 tr_[0]
  Trie01() { tr_.push_back({}); }

  /**
   * @brief 插入一个 T 类型整数（取其低 BITS 位）。
   * @param x 待插入整数
   */
  void insert(T x) {
    int u = 0;
    for (int b = BITS - 1; b >= 0; --b) {
      int id = int((x >> b) & 1);
      if (tr_[u].nxt[id] == 0) {
        tr_[u].nxt[id] = (int)tr_.size();
        tr_.push_back(Node{});
      }
      u = tr_[u].nxt[id];
      ++tr_[u].cnt;
    }
  }

  /**
   * @brief 查询与 x 异或最大值 \f$\max_y (x \oplus y)\f$，其中 y 遍历已插入元素。
   * @param x 查询值
   * @return 最大异或值
   */
  T xor_max(T x) const {
    int u = 0;
    T r = 0;
    for (int b = BITS - 1; b >= 0; --b) {
      int want = int(((x >> b) & 1) ^ 1);
      if (tr_[u].nxt[want]) {
        r |= (T(1) << b);
        u = tr_[u].nxt[want];
      } else {
        u = tr_[u].nxt[want ^ 1];
      }
    }
    return r;
  }
};

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_TRIE_HPP
