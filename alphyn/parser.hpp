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
#include "ct_parser.hpp"

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      /// \brief The class of action the parser can do when an error is encountered
      /// please note that those options only applies if the parsing is done at runtime.
      enum class on_parse_error
      {
        throw_exception,        ///< \brief An exception (std::runtime_error) is throw
        print_message,          ///< \brief A message is printed on stderr. Implies throw_exception.
        call_error_handler,     ///< \brief An error handler (located in SyntaxClass::on_parse_error) is called.
                                /// The on_parse_error must have the following def:
                                /// \code template<typename ReturnType> ReturnType on_parse_error(const char *s, size_t index); \endcode
        call_advanced_error_handler,     ///< \brief An (advanced) error handler (located in SyntaxClass::on_parse_error) is called.
                                /// The on_parse_error must have the following def:
                                /// \code template<typename ReturnType> ReturnType on_parse_error(const char *str, size_t start_index, uts_t &stack, lexem_list<SyntaxClass> &current_token); \endcode
      };

      // this way the compiler will print shorter messages (the state thing that is present in the "normal" could be HUGE)
      // this may help to debug things when the parser fails.
      template<typename TypeT, TypeT Type, typename Value>
      struct simplified_stack
      {
        static constexpr TypeT type = Type;
        using value = Value;
      };

      /// \brief The Alphyn parser
      template<typename SyntaxClass, on_parse_error OnErrAct = on_parse_error::throw_exception>
      class parser
      {
        private:
          using automaton = typename grammar_tools<SyntaxClass>::lr1_automaton;

        public:
          using type_t = typename SyntaxClass::token_type::type_t;
          using automaton_list = typename automaton::as_type_list;
          using uts_t = tuple_stack<SyntaxClass, automaton_list::size, type_t, typename SyntaxClass::grammar::return_type_list>;

        private: // compile-time
          /// \brief This way, you can use this parser to construct complex \b **types** !
          /// The result is a type, not a value
          template<const char *String, size_t Index>
          class _ct_parse_string
          {
            private:
              using rec_parser = ct_parser_rec<SyntaxClass, automaton, typename lexer<SyntaxClass>::template ct_lexem_list<String, Index>, ct::type_list<>>;

              template<typename X> using make_simplified_stack = simplified_stack<type_t, X::type, typename X::value>;

              // This way, the stack is "printed" in the error trace
              template<typename Stack>
              struct stack_check
              {
                // NOTE: If you see an error here, it may be caused by the ct parser itself. It's a quite difficult thing to debug
                // mostly if your types, when formatted correctly (replacing <> by {}) takes more than 3000 lines.
                // So as I didn't fully tested every single cases of the ct parser (simply verified that it would gives the same results
                // as the "normal" (value) parser

                // The stack is empty, meaning that the first token has been rejected.
                static_assert(Stack::size != 0, "the parser hasn't consumed the first token (it may be invalid or unexpected as a first token)");

                // The stack has not been reduced to one single entry.
                // Either the parser hasn't consumed the full token list (if the last (first) entry in the stack is a token (typed_lexem_list<>)
                // The parser failed to consume the next token) or the parser has consumed the whole input but failed to reduce the stack into the
                // initial non-terminal. (there's an error somewhere in your input, but that error didn't resulted in an invalid token or a serious
                // enough error that would have caused the parser to not consume the whole input).
                static_assert(Stack::size <= 1, "the parser hasn't consumed its full stack: this may be caused by a reduction error");

                // Typically trigger if a previous static_assert has also triggered.
                // If not, this mean that the parser has failed to reduce the expression to the initial non-terminal.
                // This may either indicate a problem in your grammar (if the whole input has been consumed, all the input has been reduced to a single
                // non-terminal but the parser can't find a way to reduce it furthermore) or in your input (you may either have consumed one token of input
                // (in that case the stack is only a typed_lexem_list<>) or you have a valid input that is in fact valid as a subexpression not valid as a whole expression
                // (in that case the stack is only a non-terminal).
                static_assert(Stack::front::type != SyntaxClass::grammar::start_rule, "the parser failed to reduce the expression to the initial production rule (reduction error)");

                using stack = Stack;
              };
              using stack = typename stack_check<typename rec_parser::stack::template direct_for_each<make_simplified_stack>>::stack;

            public:
              using result = typename stack::front::value;
          };

        public:
          /// \brief Parse the string and lay a type, this way you can use a nice and easy-to-write language to create nasty and hardcore C++ types.
          /// This is why alphin is a compile-time parser, and it fully achieve its goal in that ct_parse_string<> thing.
          /// If you want, you may even use a kind of C-subset to generate C++ types. It's possible.
          /// Your grammar may need some modifications to be supported by the ct parser (and a ct grammar may not be used by a value parser),
          /// this because the function attributes used by the standard parser may return complex types than can't be placed as template parameters
          /// (there's only support for integers/pointers/... values but not floating points values, and no other type instance).
          /// This parser is to be used if you want to output types, not values.
          /// \see parse_string()
          template<const char *String, size_t Index = 0>
          using ct_parse_string = typename _ct_parse_string<String, Index>::result;

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
          /// \brief Call (or not) an handler
          template<typename ReturnType, on_parse_error OPE> struct _on_error_switcher
          {
            static ReturnType call_handler(const char *, size_t, uts_t &, lexem_list<SyntaxClass> &)
            {
              return ReturnType();
            }
          };

          /// \brief Called when an error occurs
          template<typename ReturnType>
          static ReturnType on_error(const char *str, size_t start_index, uts_t &stack, lexem_list<SyntaxClass> &ll)
          {
            if (OnErrAct == on_parse_error::print_message)
              on_error_print_message(str, start_index, stack, ll);
            if (OnErrAct == on_parse_error::print_message || OnErrAct == on_parse_error::throw_exception)
              throw std::runtime_error(std::string("alphyn::parse_string: could not parse the string") /* + ...*/);

            if (OnErrAct == on_parse_error::call_error_handler)
              return _on_error_switcher<ReturnType, OnErrAct>::call_handler(str, start_index, stack, ll);
          };

          /// \brief Call a "simple" handler
          template<typename ReturnType>
          struct _on_error_switcher<ReturnType, on_parse_error::call_error_handler>
          {
            static ReturnType call_handler(const char *string, size_t index, uts_t &, lexem_list<SyntaxClass> &)
            {
              return SyntaxClass::template on_parse_error<ReturnType>(string, index);
            }
          };

          /// \brief Call a more advanced handler
          template<typename ReturnType>
          struct _on_error_switcher<ReturnType, on_parse_error::call_advanced_error_handler>
          {
            static ReturnType call_handler(const char *string, size_t index, uts_t &stack, lexem_list<SyntaxClass> &ll)
            {
              return SyntaxClass::template on_parse_error<ReturnType>(string, index, stack, ll);
            }
          };

          /// \brief Print an error message
          static void on_error_print_message(const char *str, size_t start_index, uts_t &stack, lexem_list<SyntaxClass> &ll)
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
                std::cerr << "All the tokens have been consumed,\nit looks like your string contains some invalid formation,\nsomewhere\n";
            }
            catch (...) {}
            if (stack.size())
            {
              std::cout << "stack: \n";
              for (size_t i = 0; i < stack.size(); ++i)
                std::cout << "       " << SyntaxClass::get_name_for_token_type(stack.get_type(i)) << '\n';
            }
            else
              std::cout << "[EMPTY STACK]" << std::endl;
            std::cerr << " -- -- SYNTAX ERROR -- --\n" << std::endl;
          }
      };
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_344197126521316_132228520__PARSER_HPP__2___*/