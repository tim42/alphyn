//
// file : lexer_skip.hpp
// in : file:///home/tim/projects/alphyn/alphyn/lexer_skip.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Mon Feb 22 2016 19:41:44 GMT+0100 (CET)
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