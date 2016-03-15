//
// file : lexer_syntax.hpp
// in : file:///home/tim/projects/alphyn/alphyn/lexer_syntax.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Mon Feb 22 2016 17:56:16 GMT+0100 (CET)
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

#ifndef __N_25402380079012009_481021210__LEXER_SYNTAX_HPP__
# define __N_25402380079012009_481021210__LEXER_SYNTAX_HPP__

#include "matcher.hpp"

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      /// \brief A syntactic unit (aka, \code "+"  { return PLUS;       } \endcode in FLex)
      /// \param Matched should be one of the following classes (in matcher.hpp): regexp< regexp_string >, string< string_to_match >, letter< 'x', 'y', 'z', ... >
      /// \param Function is the binding function to call when transforming the string into a token.
      ///                 it takes as parameters the string to tokenize, the start index and the end index.
      ///                 it returns the generated token.
      template<typename Matcher, typename TokenType, TokenType(*Function)(const char *, size_t, size_t)>
      struct syntactic_unit
      {
        syntactic_unit() = delete;

        using token_type = TokenType;

        /// \brief Return the end index of the match or -1.
        constexpr static inline long match(const char *s, long index)
        {
          return Matcher::match(s, index);
        }

        /// \brief Generate a token from a range in s
        constexpr static inline TokenType generate_token(const char *s, long start_index, long end_index)
        {
          return Function(s, start_index, end_index);
        }
      };

      /// \brief The syntax, as seen by the parser
      /// \param Units is simply some syntactic_unit<>
      template<typename... Units>
      class lexical_syntax
      {
        public:
          lexical_syntax() = delete;

          /// \brief Generate one token, advancing end_index (or setting it to -1 if something goes wrong)
          template<typename SyntaxClass>
          inline static constexpr typename SyntaxClass::token_type get_token(const char *s, long start_index, long &end_index)
          {
            return get_token_rec<SyntaxClass, Units...>(s, start_index, end_index);
          }

          /// \brief Generate one token
          template<typename SyntaxClass>
          inline static constexpr typename SyntaxClass::token_type get_token(const char *s, long start_index)
          {
            long end_index = 0;
            return get_token_rec<SyntaxClass, Units...>(s, start_index, end_index);
          }

          /// \brief Get the end index
          template<typename SyntaxClass>
          inline static constexpr long get_end_index(const char *s, long start_index)
          {
            return get_end_index_rec<SyntaxClass, Units...>(s, start_index);
          }

        private:
          /// \brief Recursively matches rules until something works or everything fails.
          template<typename SyntaxClass, typename ItUnit, typename... ItUnits>
          inline static constexpr typename SyntaxClass::token_type get_token_rec(const char *s, long start_index, long &end_index)
          {
            end_index = ItUnit::match(s, start_index);
            if (end_index != -1)
              return ItUnit::generate_token(s, start_index, end_index);
            return get_token_rec<SyntaxClass, ItUnits...>(s, start_index, end_index);
          }

          /// \brief Called when everything else fails: it returns an invalid token
          template<typename SyntaxClass>
          inline static constexpr typename SyntaxClass::token_type get_token_rec(const char *s, long start_index, long &end_index)
          {
            end_index = -1;
            return SyntaxClass::token_type::generate_invalid_token(s, start_index);
          }

          /// \brief Recursively matches rules until something works or everything fails.
          template<typename SyntaxClass, typename ItUnit, typename... ItUnits>
          inline static constexpr long get_end_index_rec(const char *s, long start_index)
          {
            const long end_index = ItUnit::match(s, start_index);
            if (end_index != -1)
              return end_index;
            return get_end_index_rec<SyntaxClass, ItUnits...>(s, start_index);
          }

          /// \brief Called when everything else fails: it returns an invalid end index
          template<typename SyntaxClass>
          inline static constexpr long get_end_index_rec(const char *, long)
          {
            return -1;
          }
      };
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_25402380079012009_481021210__LEXER_SYNTAX_HPP__*/
