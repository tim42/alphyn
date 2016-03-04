//
// file : lexer_skip.hpp
// in : file:///home/tim/projects/alphyn/alphyn/lexer_skip.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Mon Feb 22 2016 19:41:44 GMT+0100 (CET)
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

#ifndef __N_28232218622709829606_2163930939__LEXER_SKIP_HPP__
# define __N_28232218622709829606_2163930939__LEXER_SKIP_HPP__

#include "lexer_syntax.hpp"

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      namespace internal
      {
        /// \brief The skipper "token" class
        struct skipper_token_type
        {
          bool end;

          static constexpr skipper_token_type generate_invalid_token(const char *, size_t)
          {
            return skipper_token_type { true };
          }
        };

        /// \brief The skipper "token generator" func
        inline static constexpr skipper_token_type skipper_func(const char *, size_t, size_t)
        {
          return skipper_token_type { false };
        }

        /// \brief The skipper "syntax" class
        struct skipper_syntax_class { using token_type = skipper_token_type; };
      } // namespace internal

      /// \brief A simpler syntactic unit that just skip things
      template<typename Matcher>
      using skip_unit = syntactic_unit<Matcher, internal::skipper_token_type, internal::skipper_func>;

      /// \brief An alias for the skipper syntax
      template<typename... Units>
      using skip_syntax = lexical_syntax<Units...>;
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_28232218622709829606_2163930939__LEXER_SKIP_HPP__*/