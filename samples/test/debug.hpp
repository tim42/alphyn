//
// file : debug.hpp
// in : file:///home/tim/projects/alphyn/samples/test/debug.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Sun Feb 28 2016 13:42:39 GMT+0100 (CET)
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

#ifndef __N_8807169394423027_1847615346__DEBUG_HPP__
# define __N_8807169394423027_1847615346__DEBUG_HPP__

#include <string>
#include <iostream>

#include <tools/ct_list.hpp>
#include <grammar_tools.hpp>

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      /// \brief Mostly used to print the automaton.
      /// SyntaxClass will need to define a static {std::string, const char *} get_name_for_token_type(token_type::type_t) {} method
      template<typename SyntaxClass>
      struct debug_printer
      {
        static std::string get_name_for_token_type(typename SyntaxClass::token_type::type_t t)
        {
          return SyntaxClass::get_name_for_token_type(t);
        }

        template<size_t Position, typename... Types>
        static void print_rule_and_pos(const ct::type_list<Types...> &)
        {
          size_t i = 0;
          NEAM_EXECUTE_PACK(
            std::cout << (i++ == Position ? ". " : "") << get_name_for_token_type(Types::value) << ' '
          );
          if (i == Position)
            std::cout << ". ";
        }
        template<typename... Types>
        static void print_lookahead(const ct::type_list<Types...> &)
        {
          size_t i = 0;
          std::cout << " {";
          NEAM_EXECUTE_PACK(
            std::cout << (i++ > 0 ? ", " : "") << get_name_for_token_type(Types::value)
          );
          std::cout << "}";
        }
        template<typename WPRS>
        static void print_wrapped_rule()
        {
          std::cout << "  " << get_name_for_token_type(WPRS::rule_name) << " -> ";
          print_rule_and_pos<WPRS::position>(typename WPRS::as_type_list());
          print_lookahead(typename WPRS::follow_set());
          std::cout << '\n';
        }

        template<typename... WPRS>
        static void print_wrapped_rule_list(const ct::type_list<WPRS...> &)
        {
          NEAM_EXECUTE_PACK(
            (print_wrapped_rule<WPRS>())
          );
        }

        template<typename... TRS>
        static void print_transitions(const ct::type_list<TRS...> &)
        {
          size_t i = 0;
          NEAM_EXECUTE_PACK(
            std::cout << (i++ > 0 ? ", " : "") << get_name_for_token_type(TRS::name)
          );
        }

        template<typename StateList, typename Edge>
        static void print_edge()
        {
          const long state_index = StateList::template get_type_index<typename Edge::state>::index;
          std::cout << get_name_for_token_type(Edge::name) << " -> S" << state_index;
        }

        template<typename StateList, typename... Edges>
        static void print_edge_list(const ct::type_list<Edges...> &)
        {
          std::cout << "  ";
          size_t i = 0;
          NEAM_EXECUTE_PACK(
            ((std::cout << (i++ > 0 ? ", " : "")), print_edge<StateList, Edges>())
          );
          std::cout << '\n';
        }

        template<typename State, typename StateList>
        static void print_state()
        {
          const long index = StateList::template get_type_index<State>::index;
          std::cout << "state " << index << ": \n";
          print_wrapped_rule_list(typename State::non_final_rules());
          print_wrapped_rule_list(typename State::final_rules());
          if (State::edges::size)
          {
            std::cout << '\n';
            print_edge_list<StateList>(typename State::edges());
          }
          std::cout << '\n';
        }

        template<typename... States>
        static void print_state_list(const ct::type_list<States...> &)
        {
          NEAM_EXECUTE_PACK(
            (print_state<States, ct::type_list<States...>>())
          );
        }

        static void print_graph()
        {
          using graph_list = typename ct::alphyn::grammar_tools<SyntaxClass>::lr1_automaton::as_type_list;

          print_state_list(graph_list());
        }
      };
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_8807169394423027_1847615346__DEBUG_HPP__*/