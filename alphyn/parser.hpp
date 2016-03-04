//
// file : parser.hpp (2)
// in : file:///home/tim/projects/alphyn/alphyn/parser.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Sun Feb 28 2016 14:05:18 GMT+0100 (CET)
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

#ifndef __N_344197126521316_132228520__PARSER_HPP__2___
# define __N_344197126521316_132228520__PARSER_HPP__2___

#include "lexer.hpp"
#include "grammar.hpp"
#include "grammar_tools.hpp"
#include "parser_tools.hpp"

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      /// \brief The Alphyn parser
      template<typename SyntaxClass>
      class parser
      {
        private:
          using automaton = typename grammar_tools<SyntaxClass>::lr1_automaton;

        public:
          using automaton_list = typename automaton::as_type_list;
          using uts_t = union_tuple_stack<automaton_list::size, typename SyntaxClass::token_type::type_t, typename SyntaxClass::grammar::return_type_list>;

        private: // compile-time
          /// \brief Should be done in another classy way (a way full of template and types),
          /// But why the pain of doing a constexpr parser if I can't use it this way ?.
          template<const char *String, size_t Index, typename ReturnType>
          class _ct_parse_string
          {
            private:
              static constexpr ReturnType parse_string()
              {
                uts_t stack = uts_t();
                lexem_list<SyntaxClass> ll = lexer<SyntaxClass>::get_lazy_lexer(String, Index);
                parser_state<SyntaxClass, automaton>::rec_parse(stack, ll);
                if (stack.size() == 1 && stack.get_top_type() == SyntaxClass::grammar::start_rule)
                  return stack.template get<ReturnType>();
                return ReturnType();
              }
              // sorry.
              static constexpr bool can_parse_string()
              {
                uts_t stack = uts_t();
                lexem_list<SyntaxClass> ll = lexer<SyntaxClass>::get_lazy_lexer(String, Index);
                parser_state<SyntaxClass, automaton>::rec_parse(stack, ll);
                if (stack.size() == 1 && stack.get_top_type() == SyntaxClass::grammar::start_rule)
                  return true;
                return false;
              }

              static_assert(can_parse_string(), "Could not parse the string :(");

            public:
              static constexpr ReturnType result = parse_string();
          };

        public:
          /// \brief parse the string and set the result value in a static constexpr attribute named result
          /// It fails the compilation if something goes wrong with an explicit message saying that the string can't be parsed.
          /// ( parse_string(), used in a compile-time context will also fails the compilation, but the message won't be explicit)
          /// \see parse_string()
          template<typename ReturnType, const char *String, size_t Index = 0>
          using ct_parse_string = _ct_parse_string<String, Index, ReturnType>;

          /// \brief parse the string and return the result value
          /// \see ct_parse_string
          template<typename ReturnType>
          static constexpr ReturnType parse_string(const char *str, size_t start_index = 0)
          {
            uts_t stack = uts_t();
            lexem_list<SyntaxClass> ll = lexer<SyntaxClass>::get_lazy_lexer(str, start_index);
            parser_state<SyntaxClass, automaton>::rec_parse(stack, ll);
            const bool has_failed = !(stack.size() == 1 && stack.get_top_type() == SyntaxClass::grammar::start_rule);
            return !has_failed ? stack.template get<ReturnType>() :

            // The parser is unable to parse the string.
            // If you see a compilation error here, it's because you're trying to use this function at compile-time
            // on an invalid string / with an invalid grammar. When used at runtime, it either print something,
            // call a function, or even throw, depending on the settings in the SyntaxClass
            on_error<ReturnType>(str, start_index, stack, ll);
          }

        private:
          template<typename ReturnType>
          static ReturnType on_error(const char *str, size_t start_index, uts_t &stack, lexem_list<SyntaxClass> &ll)
          {
            std::cerr << "\n -- -- SYNTAX ERROR -- --" << std::endl;
            if (stack.size())
              std::cerr << "top type on the stack: " << SyntaxClass::get_name_for_token_type(stack.get_top_type()) << std::endl;
            std::cerr << "current token: " << SyntaxClass::get_name_for_token_type(ll.get_token().type) << '\n';
            try
            {
              auto toptk = ll.get_token();
              std::cerr << "here: " << str + start_index << "\n";
              std::cerr << "      " << std::string(toptk.start_index - start_index, ' ') << '^';
              if (toptk.end_index != size_t(-1) && (toptk.end_index != toptk.start_index))
                std::cerr << std::string(toptk.end_index - toptk.start_index - 1, '~');
              else if (toptk.end_index == size_t(-1))
                std::cerr << std::string(strlen(str) - start_index - toptk.start_index - 1, '~');
              std::cerr << '\n';
              if (ll.is_last() && ll.get_token().is_valid())
                std::cerr << "All the tokens has been consumed,\nit looks like your string contains some invalid formation,\nsomewhere\n";
            }
            catch (...) {}
//             if (stack.size())
//             {
//               std::cout << "stack: \n";
//               for (size_t i = 0; i < stack.size(); ++i)
//                 std::cout << "       " << SyntaxClass::get_name_for_token_type(stack.get_type(i)) << '\n';
//             }
//             else
//               std::cout << "[EMPTY STACK]" << std::endl;
            std::cerr << " -- -- SYNTAX ERROR -- --\n" << std::endl;

            throw std::runtime_error(std::string("alphyn::parse_string: could not parse the string") /* + ...*/);
          };
      };
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_344197126521316_132228520__PARSER_HPP__2___*/