//
// file : matcher.hpp
// in : file:///home/tim/projects/alphyn/alphyn/matcher.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Mon Feb 22 2016 18:19:28 GMT+0100 (CET)
//
//
// Copyright (C) 2016 Timothée Feuillet
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#ifndef __N_8759293292066131115_1257610498__MATCHER_HPP__
# define __N_8759293292066131115_1257610498__MATCHER_HPP__

#include <tools/ct_string.hpp>
#include <tools/regexp/regexp.hpp>
#include <tools/execute_pack.hpp>

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      /// \brief Matches a regular expression
      template<const char *RegExpString, size_t StartIndex = 0, size_t EndIndex = size_t(-1)>
      struct regexp
      {
        static constexpr size_t re_string_len = ct::strlen(RegExpString);
        static constexpr size_t stored_string_size = (EndIndex > re_string_len ? re_string_len : EndIndex) - StartIndex;

        template<size_t... Indexes>
        struct string_storage
        {
          static constexpr char reg_exp_string[stored_string_size + 1] = {RegExpString[StartIndex + Indexes]..., '\0'};
        };
        template<size_t Current, size_t... Indexes> struct string_holder : public string_holder<Current - 1, Current - 1, Indexes...> {};
        template<size_t... Indexes> struct string_holder<0, Indexes...> : public string_storage<Indexes...> {};

        using regtype = neam::ct::regexp<string_holder<stored_string_size>::reg_exp_string>;

        static constexpr long match(const char *s, long index)
        {
          return regtype::match(s, index);
        }
      };

      // "normal" implementation of the regexp matcher (no storage overhead)
      template<const char *RegExpString>
      struct regexp<RegExpString, 0, size_t(-1)>
      {
        using regtype = neam::ct::regexp<RegExpString>;

        static constexpr long match(const char *s, long index)
        {
          return regtype::match(s, index);
        }
      };

      /// \brief Matches a string
      /// \note The string must matches whole
      template<const char *String, size_t StartIndex = 0, size_t EndIndex = ct::strlen(String)>
      struct string
      {
        static constexpr size_t string_len = ct::strlen(String);
        static constexpr size_t end_index = (EndIndex > string_len ? string_len : EndIndex);

        static constexpr long match(const char *s, long index)
        {
          size_t i = index;
          size_t j = StartIndex;
          for (; s[i] != '\0' && j < end_index && String[j] == s[i]; ++i, ++j);
          if (j < end_index)
            return -1;
          return i;
        }
      };

      /// \brief Matches a single letter (one of the list)
      template<const char... Char>
      struct letter
      {
        static constexpr long match(const char *s, long index)
        {
          const char c = s[index];
          bool matched = false;
          NEAM_EXECUTE_PACK((c == Char) ? (matched = true) : false);
          return matched ? index + 1 : -1;
        }
      };
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_8759293292066131115_1257610498__MATCHER_HPP__*/