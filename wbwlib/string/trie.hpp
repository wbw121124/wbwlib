#ifndef WBWLIB_STR_TRIE_HPP
#define WBWLIB_STR_TRIE_HPP

/**
 * @file trie.hpp
 * @brief 字典树（Trie），含 01-Trie 最大异或。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：插入/查询 O(len) 或 O(位数)。
 *
 * 用法：
 *   wbwlib::str::Trie tr;
 *   tr.insert("abc"); tr.insert("abd");
 *   tr.count_prefix("ab");          // 以 "ab" 为前缀的串数
 *   tr.search("abd")==1;            // 完整串
 *
 * 01-Trie：xor_max(x) 返回与 x 异或结果最大值对应的元素的异或值/可取最大异或和
 *   wbwlib::str::Trie01 t;
 *   t.insert(5); int best = t.xor_max(8);    // 返回最大异或值
 */

#include <string>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace str {

// ================= 字符串 Trie =================
class Trie {
  static const int ALPHA = 26;
  struct Node { int nxt[ALPHA]; int cnt, end; };
  std::vector<Node> tr_;

 public:
  Trie() { tr_.push_back({}); }   // 根节点 tr_[0]；Node 大括号 value-init 归零

  void insert(const std::string& s) {
    int u = 0;
    for (char c : s) {
      int id = c - 'a';
      if (tr_[u].nxt[id] == 0) {
        tr_[u].nxt[id] = (int)tr_.size();
        tr_.push_back(Node{});
      }
      u = tr_[u].nxt[id];
      ++tr_[u].cnt;
    }
    ++tr_[u].end;
  }

  /// 以 s 为完整串的次数
  int count(const std::string& s) const {
    int u = 0;
    for (char c : s) {
      int id = c - 'a';
      if (tr_[u].nxt[id] == 0) return 0;
      u = tr_[u].nxt[id];
    }
    return tr_[u].end;
  }

  /// 以 s 为前缀的串数目
  int count_prefix(const std::string& s) const {
    int u = 0;
    for (char c : s) {
      int id = c - 'a';
      if (tr_[u].nxt[id] == 0) return 0;
      u = tr_[u].nxt[id];
    }
    return tr_[u].cnt;
  }

  bool search(const std::string& s) const { return count(s) > 0; }
};

// ================= 01-Trie（最大异或） =================
class Trie01 {
  static const int BITS = 32;       // 适用 32 位；64 位可改模板参数
  struct Node { int nxt[2]; int cnt; };
  std::vector<Node> tr_;

 public:
  Trie01() { tr_.push_back({}); }

  void insert(u32 x) {
    int u = 0;
    for (int b = BITS - 1; b >= 0; --b) {
      int id = (x >> b) & 1;
      if (tr_[u].nxt[id] == 0) {
        tr_[u].nxt[id] = (int)tr_.size();
        tr_.push_back(Node{});
      }
      u = tr_[u].nxt[id];
      ++tr_[u].cnt;
    }
  }

  /// 返回与 x 异或的最大值（即 max_y (x^y)，y 为已插入元素）
  u32 xor_max(u32 x) const {
    int u = 0;
    u32 r = 0;
    for (int b = BITS - 1; b >= 0; --b) {
      int want = ((x >> b) & 1) ^ 1;
      if (tr_[u].nxt[want]) {
        r |= (1u << b);
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