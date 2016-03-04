//
// file : lexer.hpp (2)
// in : file:///home/tim/projects/alphyn/alphyn/lexer.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Mon Feb 22 2016 17:27:41 GMT+0100 (CET)
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

#ifndef __N_2237319796124805534_24511980__LEXER_HPP__2___
# define __N_2237319796124805534_24511980__LEXER_HPP__2___

#include "lexer_syntax.hpp"
#include "lexer_skip.hpp"
#include "white_space_skipper.hpp"
#include "lexem_list.hpp"

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      /// \brief A simple lexer.
      /// It has skip rules and conversion rules.
      /// \note If you plan to use the alphyn's parser, you should stick to use simple rules.
      /// \note The creation of a token should not depends of a previous token
      ///
      /// The lexer is made to be inlined/optimized, and thus for simple grammar it may be extremely fast with a very low count of instruction.
      /// The more you add syntactic_units and complex regular expressions, the bigger the output assembly file will be.
      /// It uses a regular expression "engine" that has the same goal:
      /// With a smart enough compiler it will merge the lexer and the regular expression, optimizing both of them at the same time.
      /// Just remember to activate inlining (with a high limit, if possible)
      ///
      /// The SyntaxClass class must have the following properties:
      ///  - token_type which is the the type of token to generate
      ///  - invalid_token (which is a static member, kudos if it's constexpr)
      ///  - lexical_syntax, of type lexical_syntax<>, which is the syntax used by the lexer to generate tokens
      ///  - skipper, of type skip_syntax<>, which indicate to the lexer what is to skip
      ///             if you just want to skip white spaces ( ,\n,\t) simply do \code using skipper = neam::ct::alphyn::white_space_skipper;\endcode
      ///             if you don't want to skip anything, just do \code using skipper = neam::ct::alphyn::skip_syntax<>;\endcode
      ///  - lexer, an alias to this neam::ct::alphyn::lexer< SyntaxClass >.
      template<typename SyntaxClass>
      class lexer
      {
        public: // types
          using token_type = typename SyntaxClass::token_type;
          using syntax_type = typename SyntaxClass::lexical_syntax;
          using skipper_type = typename SyntaxClass::skipper;
          using syntax_class_type = SyntaxClass;

          using lazy_lexem_list = lexem_list<SyntaxClass>;

          /// \brief This one is special: it create a by-type lexeme list (types are used in place of instances/functions calls)
          /// \note It will fail to compile if the string contains invalid tokens
          /// \see get_lazy_lexer()
          template<const char *Str, size_t Index = 0>
          using ct_lexem_list = typed_lexem_list<SyntaxClass, Str, Index>;

        public:
          lexer() = delete; // you cannot instanciate this class.

          /// \brief The entry point of the lexer.
          /// It return a lazy-constructed lexem_list (it only scans one token at a time, when needed)
          /// \see ct_lexem_list
          static constexpr inline lazy_lexem_list get_lazy_lexer(const char *s, size_t start_index = 0)
          {
            return lazy_lexem_list(s, start_index);
          }

          /// \brief Return a single token, updates end_index (to either -1 if something fails, or the new end_index)
          /// \note You may never call this function
          static constexpr inline token_type get_token(const char *s, long start_index, long &end_index)
          {
            return syntax_type::template get_token<SyntaxClass>(s, skip(s, start_index), end_index);
          }

          /// \brief Return a single token
          /// \note You may never call this function
          static constexpr inline token_type get_token(const char *s, long start_index)
          {
            return syntax_type::template get_token<SyntaxClass>(s, skip(s, start_index));
          }

          /// \brief Return the end index
          /// \note You may never call this function
          static constexpr inline long get_end_index(const char *s, long start_index)
          {
            return syntax_type::template get_end_index<SyntaxClass>(s, skip(s, start_index));
          }

        private:
          /// \brief skip tokens
          static constexpr inline long skip(const char *s, long start_index)
          {
            long last_index = start_index;
            long r = 0;
            while (!skipper_type::template get_token<internal::skipper_syntax_class>(s, last_index, r).end)
              last_index = r;
            return last_index;
          }
      };
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_2237319796124805534_24511980__LEXER_HPP__2___*/