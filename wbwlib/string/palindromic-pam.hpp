#ifndef WBWLIB_STR_PALINDROMIC_PAM_HPP
#define WBWLIB_STR_PALINDROMIC_PAM_HPP

/**
 * @file palindromic-pam.hpp
 * @brief 回文自动机（回文树）PAM：模板化字符类型 / 字母表 / 字符映射。
 *
 * @par 依赖
 * wbwlib/core/base.hpp, wbwlib/string/char-map.hpp
 *
 * @par 复杂度
 * 构建 O(n)，每个字符均摊 O(1)。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::str::PAM<> pam;                // 默认：小写字母
 *   pam.build("abacaba");
 *   pam.distinct();                        // 本质不同回文子串数
 * @endcode
 */

/**
 * @file palindromic-pam.hpp
 * @brief 回文自动机（回文树）PAM。
 *
 * @par 依赖
 * wbwlib/core/base.hpp
 *
 * @par 复杂度
 * 构建 O(n)，节点数 ≤ n+2。
 *
 * 节点 0 表示奇数长回文（len = -1），节点 1 表示偶数长回文（len = 0）。
 * 每个节点代表一个本质不同的回文串。
 *
 * @par 示例
 * @code{.cpp}
 *   wbwlib::str::PAM pam;
 *   pam.build(s);                 // 构建+统计次数
 *   i64 d = pam.distinct();       // 本质不同回文子串数 = sz-2
 *   i64 occ = pam.occ(node_id);   // 该回文出现次数（build 后有效）
 *   int l = pam.len(node_id);     // 回文长度
 * @endcode
 */

#include <string>
#include <vector>
#include <algorithm>
#include "wbwlib/core/base.hpp"
#include "wbwlib/string/char-map.hpp"

namespace wbwlib {
namespace str {

/**
 * @brief 回文自动机（回文树）PAM：每个节点代表一个本质不同的回文串。
 *
 * 节点 0 为奇根（len = -1），节点 1 为偶根（len = 0）。
 * @tparam Char    字符类型（默认 char）
 * @tparam ALPHA   字母表大小（默认 26）
 * @tparam CharMap 字符映射策略（默认小写字母 LowerCharMap）
 */
template<class Char = char, int ALPHA = 26, class CharMap = LowerCharMap>
class PAM {

 public:
  /**
   * @brief PAM 节点：表示一个回文串及其转移/失配信息。
   */
  struct Node {
    int next[ALPHA];   ///< 转移
    int len;           ///< 回文长度
    int fail;          ///< 失配指针
    i64 cnt;           ///< 出现次数（build 后为真实次数）
  };
  std::vector<Node> node_;
  int last_;
  int sz_;             ///< 节点总数
  int n_;              ///< 已插入字符数
  std::vector<int> s_; ///< 字符序列（0 基），前哨兵用 -1

  /// 构造空 PAM（等价于 reset）
  PAM() { reset(); }

  /// 重置为只含两个根节点的初始状态
  void reset() {
    node_.clear();
    node_.push_back({{}, -1, 0, 0});  // 节点 0：奇根 len=-1，fail 自指
    node_.push_back({ {}, 0, 0, 0});  // 节点 1：偶根 len=0
    last_ = 1;
    sz_ = 2;
    n_ = 0;
    s_.clear();
    s_.push_back(-1);
  }

  /// 沿 fail 追溯，直到能匹配当前位置
  int get_fail(int x) {
    while (s_[n_ - node_[x].len - 1] != s_[n_]) x = node_[x].fail;
    return x;
  }

  /**
   * @brief 插入字符 c（0 基），返回新产生节点的编号（若未产生新回文则返回被命中的已有节点）。
   * @param c 字符编码（0..25）
   * @return 新回文串对应节点编号
   */
  int extend(int c) {
    s_.push_back(c);
    ++n_;
    int cur = get_fail(last_);
    int now;
    if (node_[cur].next[c] == 0) {
      now = sz_++;
      node_.push_back({});
      node_[now].len = node_[cur].len + 2;
      if (node_[now].len == 1) {
        node_[now].fail = 1;
      } else {
        int f = get_fail(node_[cur].fail);
        node_[now].fail = node_[f].next[c];
      }
      node_[cur].next[c] = now;
    } else {
      now = node_[cur].next[c];
    }
    last_ = now;
    return now;
  }

  /**
   * @brief 构建回文自动机：逐个插入字符并计数，最后按 len 降序把 cnt 沿 fail 累加得到真实出现次数。
   * @param s 输入字符串（字符集须被 CharMap::valid 覆盖）
   */
  void build(const std::basic_string<Char>& s) {
    for (Char ch : s) {
      int nd = extend(CharMap::id(ch));
      ++node_[nd].cnt;          // 该最长回文计数 +1
    }
    // 拓扑累加：按 len 降序，把 cnt 累加到 fail 上
    std::vector<int> order(sz_);
    for (int i = 0; i < sz_; ++i) order[i] = i;
    std::sort(order.begin(), order.end(),
              [&](int a, int b) { return node_[a].len > node_[b].len; });
    for (int v : order)
      if (v >= 2) node_[node_[v].fail].cnt += node_[v].cnt;
  }

  /// 返回节点总数（含两个根）
  int size() const { return sz_; }
  /// 返回本质不同回文子串数（= sz - 2）
  i64 distinct() const { return (i64)sz_ - 2; }
  /// 返回编号 id 节点的回文长度
  int len(int id) const { return node_[id].len; }
  /// 返回编号 id 回文串的出现次数（build 后有效）
  i64 occ(int id) const { return node_[id].cnt; }
};

} // namespace str
} // namespace wbwlib

#endif // WBWLIB_STR_PALINDROMIC_PAM_HPP