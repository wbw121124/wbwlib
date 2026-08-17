#ifndef WBWLIB_STRING_STRING_HPP
#define WBWLIB_STRING_STRING_HPP

/**
 * @file string/string.hpp
 * @brief string 模块聚合头：一次引入本模块全部头文件。
 *
 * @par 依赖
 * 各子头文件自带依赖（include guard 保证幂等）。
 *
 * @par 示例
 * @code{.cpp}
 *   #include <wbwlib/string/string.hpp>
 * @endcode
 */












#include "wbwlib/string/ac-automaton.hpp"
#include "wbwlib/string/char-map.hpp"
#include "wbwlib/string/kmp.hpp"
#include "wbwlib/string/manacher.hpp"
#include "wbwlib/string/minimal-string.hpp"
#include "wbwlib/string/palindromic-pam.hpp"
#include "wbwlib/string/rolling-hash.hpp"
#include "wbwlib/string/suffix-array.hpp"
#include "wbwlib/string/suffix-automaton.hpp"
#include "wbwlib/string/trie.hpp"
#include "wbwlib/string/z.hpp"

#endif // WBWLIB_STRING_STRING_HPP
