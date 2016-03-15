//
// file : matcher.hpp
// in : file:///home/tim/projects/alphyn/alphyn/matcher.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Mon Feb 22 2016 18:19:28 GMT+0100 (CET)
//
//
// Copyright (c) 2016 Timothée Feuillet
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
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