//  _   _                             __    _____
// | \ | |                           / _|  / ____|_     _
// |  \| | __ _ _ __ ___   ___  ___ | |_  | |   _| |_ _| |_
// | . ` |/ _` | '_ ` _ \ / _ \/ _ \|  _| | |  |_   _|_   _|
// | |\  | (_| | | | | | |  __/ (_) | |   | |____|_|   |_|
// |_| \_|\__,_|_| |_| |_|\___|\___/|_|    \_____|
// https://github.com/Neargye/nameof
// vesion 0.5.0
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2016, 2018 - 2019 Daniil Goncharov <neargye@gmail.com>.
//
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <cstddef>
#include <type_traits>
#include <string_view>

#if !defined(NAMEOF_ENUM_MAX_SEARCH_DEPTH)
#  define NAMEOF_ENUM_MAX_SEARCH_DEPTH 256
#endif

namespace nameof {

namespace detail {

template <typename T>
struct identity {
  using type = T;
};

constexpr bool is_lexeme(char s) noexcept {
  return !((s >= '0' && s <= '9') || (s >= 'a' && s <= 'z') || (s >= 'A' && s <= 'Z') || s == '_');
}

constexpr bool is_bracket(char s) noexcept {
  return s == ')' || s == '}' || s == '>' || s == '(' || s == '{' || s == '<';
}

constexpr std::string_view pretty_name(std::string_view name, bool with_suffix) {
  for (std::size_t i = name.size(), h = 0, s = 0; i > 0; --i) {
    if (h == 0 && is_lexeme(name[i - 1]) && !is_bracket(name[i - 1])) {
      ++s;
      continue;
    }

    if (name[i - 1] == ')' || name[i - 1] == '}') {
      ++h;
      ++s;
      continue;
    } else if (name[i - 1] == '(' || name[i - 1] == '{') {
      --h;
      ++s;
      continue;
    }

    if (h == 0) {
      name.remove_suffix(s);
      break;
    } else {
      ++s;
      continue;
    }
  }

  std::size_t s = 0;
  for (std::size_t i = name.size(), h = 0; i > 0; --i) {
    if (name[i - 1] == '>') {
      ++h;
      ++s;
      continue;
    } else if (name[i - 1] == '<') {
      --h;
      ++s;
      continue;
    }

    if (h == 0) {
      break;
    } else {
      ++s;
      continue;
    }
  }

  for (std::size_t i = name.size() - s; i > 0; --i) {
    if (is_lexeme(name[i - 1])) {
      name.remove_prefix(i);
      break;
    }
  }
  name.remove_suffix(with_suffix ? 0 : s);

  return name;
}

template <typename T>
constexpr int nameof_enum_impl_() {
#if defined(__clang__)
  return sizeof(__PRETTY_FUNCTION__) - sizeof("int nameof::detail::nameof_enum_impl_() [T = ") - sizeof("]") + 1;
#elif defined(__GNUC__)
  return sizeof(__PRETTY_FUNCTION__) - sizeof("constexpr int nameof::detail::nameof_enum_impl_() [with T = ") - sizeof("]") + 1;
#elif defined(_MSC_VER)
  return sizeof(__FUNCSIG__) - sizeof("int __cdecl nameof::detail::nameof_enum_impl_<") - sizeof(">(void)") + 1;
#else
  return 0;
#endif
}

template <typename T, T V>
constexpr std::string_view nameof_enum_impl() {
#if defined(__clang__)
  const auto str = __PRETTY_FUNCTION__;
  const auto size = sizeof(__PRETTY_FUNCTION__) - 1;
  const auto prefix = sizeof("std::string_view nameof::detail::nameof_enum_impl() [T = ") + nameof_enum_impl_<T>() + sizeof("; V = ") - 2;
  const auto suffix = sizeof("]") - 1;
  return {str + prefix, size - prefix - suffix};
#elif defined(__GNUC__)
  const auto str = __PRETTY_FUNCTION__;
  const auto size = sizeof(__PRETTY_FUNCTION__) - 1;
  const auto prefix = sizeof("constexpr std::string_view nameof::detail::nameof_enum_impl() [with T = ") + nameof_enum_impl_<T>() + sizeof("; V = ");
  const auto suffix = sizeof("; std::string_view = std::basic_string_view<char>]") - 1;
  return {str + prefix, size - prefix - suffix};
#elif defined(_MSC_VER)
  const auto str = __FUNCSIG__;
  const auto size = sizeof(__FUNCSIG__) - 1;
  const auto prefix = sizeof("class std::basic_string_view<char,struct std::char_traits<char> > __cdecl nameof::detail::nameof_enum_impl<") + nameof_enum_impl_<T>();
  const auto suffix = sizeof(">(void)") - 1;
  return {str + prefix, size - prefix - suffix};
#else
  return {};
#endif
}

template <typename T, int I>
struct nameof_enum_t {
  constexpr std::string_view operator()(int value) const {
    switch (value - I) {
      case 0:
        return nameof_enum_impl<T, T{I}>();
      case 1:
        return nameof_enum_impl<T, T{I + 1}>();
      case 2:
        return nameof_enum_impl<T, T{I + 2}>();
      case 3:
        return nameof_enum_impl<T, T{I + 3}>();
      case 4:
        return nameof_enum_impl<T, T{I + 4}>();
      case 5:
        return nameof_enum_impl<T, T{I + 5}>();
      case 6:
        return nameof_enum_impl<T, T{I + 6}>();
      case 7:
        return nameof_enum_impl<T, T{I + 7}>();
      default:
        return nameof_enum_t<T, I + 8>{}(value);
    }
  }
};

template <typename T>
struct nameof_enum_t<T, NAMEOF_ENUM_MAX_SEARCH_DEPTH> {
  constexpr std::string_view operator()(int) const {
    return { "nameof_enum::out_of_range" };
  }
};

constexpr std::string_view nameof_type_impl_(std::string_view name) {
#if defined(_MSC_VER)
  if (name.size() > sizeof("enum") && name[0] == 'e' && name[1] == 'n' && name[2] == 'u' && name[3] == 'm' && name[4] == ' ') {
    name.remove_prefix(sizeof("enum"));
  }
  if (name.size() > sizeof("class") && name[0] == 'c' && name[1] == 'l' && name[2] == 'a' && name[3] == 's' && name[4] == 's' && name[5] == ' ') {
    name.remove_prefix(sizeof("class"));
  }
  if (name.size() > sizeof("struct") && name[0] == 's' && name[1] == 't' && name[2] == 'r' && name[3] == 'u' && name[4] == 'c' && name[5] == 't' && name[6] == ' ') {
    name.remove_prefix(sizeof("struct"));
  }
#endif
  while (name.back() == ' ') {
    name.remove_suffix(1);
  }

  return name;
}

template <typename T>
constexpr std::string_view nameof_type_impl() {
#if defined(__clang__)
  const auto str = __PRETTY_FUNCTION__;
  const auto size = sizeof(__PRETTY_FUNCTION__) - 1;
  const auto prefix = sizeof("std::string_view nameof::detail::nameof_type_impl() [T = nameof::detail::identity<") - 1;
  const auto suffix = sizeof(">]") - 1;
  return nameof_type_impl_({str + prefix, size - prefix - suffix});
#elif defined(__GNUC__)
  const auto str = __PRETTY_FUNCTION__;
  const auto size = sizeof(__PRETTY_FUNCTION__) - 1;
  const auto prefix = sizeof("constexpr std::string_view nameof::detail::nameof_type_impl() [with T = nameof::detail::identity<") - 1;
  const auto suffix = sizeof(">; std::string_view = std::basic_string_view<char>]") - 1;
  return nameof_type_impl_({str + prefix, size - prefix - suffix});
#elif defined(_MSC_VER)
  const auto str = __FUNCSIG__;
  const auto size = sizeof(__FUNCSIG__) - 1;
  const auto prefix = sizeof("class std::basic_string_view<char,struct std::char_traits<char> > __cdecl nameof::detail::nameof_type_impl<struct nameof::detail::identity<") - 1;
  const auto suffix = sizeof(">>(void)") - 1;
  return nameof_type_impl_({str + prefix, size - prefix - suffix});
#else
  return {};
#endif
}

} // namespace detail

template <typename T,
          typename = typename std::enable_if<!std::is_reference<T>::value>::type>
constexpr std::string_view nameof(std::string_view name, bool with_suffix = false) {
  return detail::pretty_name(name, with_suffix);
}

template <typename T,
          typename = typename std::enable_if<std::is_enum<typename std::decay<T>::type>::value>::type>
constexpr std::string_view nameof_enum(T value) {
  constexpr auto s = std::is_signed<typename std::underlying_type<typename std::decay<T>::type>::type>::value;
  const auto name = detail::nameof_enum_t<typename std::decay<T>::type, s ? -NAMEOF_ENUM_MAX_SEARCH_DEPTH : 0>{}(static_cast<int>(value));
#if defined(__clang__) || defined(_MSC_VER)
  return detail::pretty_name(name, false);
#elif defined(__GNUC__)
  return name;
#else
  return {};
#endif
}

template <typename T>
constexpr std::string_view nameof_type() {
  return detail::nameof_type_impl<detail::identity<T>>();
}

template <typename T>
constexpr std::string_view nameof_raw(std::string_view name) {
  return name;
}

} // namespace nameof

// Used to obtain the simple (unqualified) string name of a variable, member, function, enum, macros.
#define NAMEOF(...) ::nameof::nameof<decltype(__VA_ARGS__)>(#__VA_ARGS__, false)

// Used to obtain the full string name of a variable, member, function, enum, macros.
#define NAMEOF_FULL(...) ::nameof::nameof<decltype(__VA_ARGS__)>(#__VA_ARGS__, true)

// Used to obtain the raw string name of a variable, member, function, enum, macros.
#define NAMEOF_RAW(...) ::nameof::nameof_raw<decltype(__VA_ARGS__)>(#__VA_ARGS__)

// Used to obtain the simple (unqualified) string name of a enum variable.
#define NAMEOF_ENUM(...) ::nameof::nameof_enum<decltype(__VA_ARGS__)>(__VA_ARGS__)

// Used to obtain the string name of a type.
#define NAMEOF_TYPE(...) ::nameof::nameof_type<decltype(__VA_ARGS__)>()
#define NAMEOF_TYPE_T(...) ::nameof::nameof_type<__VA_ARGS__>()
