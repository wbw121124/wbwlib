#ifndef WBWLIB_STR_AC_AUTOMATON_HPP
#define WBWLIB_STR_AC_AUTOMATON_HPP

/**
 * @file ac-automaton.hpp
 * @brief AC 自动机：多模式匹配（模板化字符类型 / 字母表 / 字符映射）。
 *
 * @par 依赖
 * wbwlib/core/base.hpp, wbwlib/string/char-map.hpp
 *
 * @par 复杂度
 * 构建 O(总模式长·Σ)，匹配 O(文本长)。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::str::ACAutomaton<> ac;       // 默认：小写字母
 *   ac.insert("he"); ac.insert("she");
 *   ac.build();
 *   i64 total = ac.search("ushershe");   // 返回所有模式出现总次数
 *   // 也可遍历 fail 数组建 fail 树做树上差分（ac.fail）
 *
 *   // 自定义字符集：如大小写混合
 *   struct MixedMap { static int id(char c){ return c<='Z'? c-'A'+26 : c-'a'; } static bool valid(char c){ return (c>='a'&&c<='z')||(c>='A'&&c<='Z'); } };
 *   wbwlib::str::ACAutomaton<char, 52, MixedMap> ac2;
 * @endcode
 */

#include <queue>
#include <string>
#include <vector>
#include "wbwlib/core/base.hpp"
#include "wbwlib/string/char-map.hpp"

namespace wbwlib {
namespace str {

/**
 * @brief AC 自动机：多模式匹配，构建 fail 指针后线性扫描文本统计模式出现。
 * @tparam Char    字符类型（默认 char）
 * @tparam ALPHA   字母表大小（默认 26）
 * @tparam CharMap 字符映射策略（默认小写字母 LowerCharMap）
 */
template<class Char = char, int ALPHA = 26, class CharMap = LowerCharMap>
class ACAutomaton {
  struct Node {
    int nxt[ALPHA];
    int fail;
    int cnt;      // 以该节点为整串的模式数量
    int e2;       // fail 链累计（匹配时直接累加）
  };
  std::vector<Node> tr_;

 public:
  /// 构造空 AC 自动机：创建根节点 tr_[0]
  ACAutomaton() { tr_.push_back({}); }

  /// 失配指针数组（长度 = node_count()，用于构建 fail 树做树上差分统计）
  std::vector<int> fail;

  /// 清空所有模式并重建根节点
  void clear() { tr_.clear(); tr_.push_back({}); }

  /**
   * @brief 插入一个模式串（字符集须被 CharMap::valid 覆盖）。
   * @param s 模式串
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
    }
    ++tr_[u].cnt;
  }

  /**
   * @brief BFS 建立失配指针，并预处理各节点的 e2（fail 链上 cnt 累计）。
   */
  void build() {
    fail.assign(tr_.size(), 0);
    std::queue<int> q;
    for (int c = 0; c < ALPHA; ++c) {
      if (tr_[0].nxt[c]) {
        tr_[tr_[0].nxt[c]].fail = 0;
        q.push(tr_[0].nxt[c]);
      }
    }
    while (!q.empty()) {
      int u = q.front(); q.pop();
      fail[u] = tr_[u].fail;
      tr_[u].e2 = tr_[u].cnt + tr_[tr_[u].fail].e2;
      for (int c = 0; c < ALPHA; ++c) {
        int v = tr_[u].nxt[c];
        if (v) {
          tr_[v].fail = tr_[tr_[u].fail].nxt[c];
          q.push(v);
        } else {
          tr_[u].nxt[c] = tr_[tr_[u].fail].nxt[c];
        }
      }
    }
  }

  /**
   * @brief 扫描文本，统计所有模式串的出现总次数（每个出现各计 1）。
   * @param text 待扫描文本
   * @return 所有模式出现总次数
   */
  i64 search(const std::basic_string<Char>& text) const {
    i64 ans = 0;
    int u = 0;
    for (Char c : text) {
      int id = CharMap::id(c);
      u = tr_[u].nxt[id];
      ans += tr_[u].e2;
    }
    return ans;
  }

  /// 返回 Trie 节点总数（含根节点）
  int node_count() const { return (int)tr_.size(); }
};

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_AC_AUTOMATON_HPP