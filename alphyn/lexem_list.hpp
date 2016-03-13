//
// file : lexem_list.hpp
// in : file:///home/tim/projects/alphyn/alphyn/lexem_list.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Mon Feb 22 2016 23:09:28 GMT+0100 (CET)
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

#ifndef __N_16638100081563431728_1110126257__LEXEM_LIST_HPP__
# define __N_16638100081563431728_1110126257__LEXEM_LIST_HPP__

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      /// \brief The list of lexem/tokens (lazy generated)
      template<typename SyntaxClass>
      class lexem_list
      {
        public: // types
          using token_type = typename SyntaxClass::token_type;
          using lexer_type = typename SyntaxClass::lexer;
          using syntax_class_type = SyntaxClass;

        public:
          /// \brief constructor
          constexpr lexem_list(const char *_str, size_t _start_index)
            : str(_str), start_index(_start_index), end_index(-1),
              token(lexer_type::get_token(str, start_index, end_index))
          {}

          /// \brief copy constructor (no move, 'cause that does not mean anything)
          constexpr lexem_list(const lexem_list &o)
            : str(o.str), start_index(o.start_index), end_index(o.end_index), token(o.token)
          {}

          /// \brief Return the token of the current lexem
          constexpr const token_type &get_token() const { return token; }

          /// \brief implicit-cast into token_type
          constexpr operator const token_type &() const { return token; }

          /// \brief Return the next lexem_list entry
          constexpr lexem_list get_next() const
          {
            return lexem_list(str, ((end_index == -1) ? start_index : end_index));
          }

          /// \brief Return true if the current entry is the last entry of the list
          constexpr bool is_last() const
          {
            return (end_index == -1) || (str[start_index] == '\0');
          }

        private:
          const char *str;
          size_t start_index;
          long end_index;

          token_type token;
      };

      /// \brief The list of lexem/tokens (by types instead of instances)
      template<typename SyntaxClass, const char *Str, long InitialIndex = 0, bool Empty = false>
      class typed_lexem_list
      {
        public: // types
          using token_type = typename SyntaxClass::token_type;
          using lexer_type = typename SyntaxClass::lexer;
          using syntax_class_type = SyntaxClass;

        private: // helpers
          static constexpr long end_index = lexer_type::get_end_index(Str, InitialIndex);

        public:
          static constexpr token_type token = lexer_type::get_token(Str, InitialIndex);
          static constexpr bool is_last = ((end_index == -1) || (Str[InitialIndex] == '\0'));

        private: // check
          template<long InvalidTokenIndex>
          static constexpr bool check_token()
          {
            // The lexer has generated an invalid token 'cause it bumped on an unexpected input
            // Hopefully, you'll have the index that caused the fault, just lookup the value of InvalidTokenIndex
            // NOTE: It may be lazy evaluated by the compiler when the type is resolved
            static_assert(end_index != -1, "Invalid token in string");
            return is_last;
          }

        public:
          using next = typed_lexem_list<SyntaxClass, Str, (is_last ? InitialIndex : end_index), check_token<token.start_index>()>;
      };
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_16638100081563431728_1110126257__LEXEM_LIST_HPP__*/