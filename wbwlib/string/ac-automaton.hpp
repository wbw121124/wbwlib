#ifndef WBWLIB_STR_AC_AUTOMATON_HPP
#define WBWLIB_STR_AC_AUTOMATON_HPP

/**
 * @file ac-automaton.hpp
 * @brief AC 自动机：多模式匹配。
 *
 * 依赖：wbwlib/core/base.hpp
 *
 * 复杂度：构建 O(总模式长·Σ)，匹配 O(文本长)。
 *
 * 用法：
 *   wbwlib::str::ACAutomaton ac;
 *   ac.insert("he"); ac.insert("she");
 *   ac.build();
 *   i64 total = ac.search("ushershe");   // 返回所有模式出现总次数
 *   // 也可遍历 fail 数组建 fail 树做树上差分（ac.fail）
 */

#include <queue>
#include <string>
#include <vector>
#include "wbwlib/core/base.hpp"

namespace wbwlib {
namespace str {

class ACAutomaton {
  static const int ALPHA = 26;
  struct Node {
    int nxt[ALPHA];
    int fail;
    int cnt;      // 以该节点为整串的模式数量
    int e2;       // fail 链累计（匹配时直接累加）
  };
  std::vector<Node> tr_;

 public:
  ACAutomaton() { tr_.push_back({}); }

  /// 失配指针数组（长度 = node_count()，用于构建 fail 树做树上差分统计）
  std::vector<int> fail;

  void clear() { tr_.clear(); tr_.push_back({}); }

  void insert(const std::string& s) {
    int u = 0;
    for (char c : s) {
      int id = c - 'a';
      if (tr_[u].nxt[id] == 0) {
        tr_[u].nxt[id] = (int)tr_.size();
        tr_.push_back(Node{});
      }
      u = tr_[u].nxt[id];
    }
    ++tr_[u].cnt;
  }

  /// BFS 建立失配指针并预处理 e2
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

  /// 扫描文本，返回所有模式出现总次数（每个出现各计 1）
  i64 search(const std::string& text) const {
    i64 ans = 0;
    int u = 0;
    for (char c : text) {
      int id = c - 'a';
      u = tr_[u].nxt[id];
      ans += tr_[u].e2;
    }
    return ans;
  }

  int node_count() const { return (int)tr_.size(); }
};

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_AC_AUTOMATON_HPP