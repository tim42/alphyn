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

#include <tools/regexp/regexp.hpp>
#include <tools/execute_pack.hpp>

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      /// \brief Matches a regular expression
      template<const char *RegExpString>
      struct regexp
      {
        using regtype = neam::ct::regexp<RegExpString>;

        static constexpr long match(const char *s, long index)
        {
          return regtype::match(s, index);
        }
      };

      /// \brief Matches a string
      /// \note The string must matches whole
      template<const char *String>
      struct string
      {
        static constexpr long match(const char *s, long index)
        {
          long ret = 0;
          for (; s[ret] && String[ret]; ++ret)
          {
            if (s[ret] != String[ret])
              return -1;
          }

          if (s[ret] != String[ret])
              return -1;
          return index + ret;
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