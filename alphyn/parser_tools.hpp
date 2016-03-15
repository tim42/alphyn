//
// file : parser_tools.hpp
// in : file:///home/tim/projects/alphyn/alphyn/parser_tools.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Sun Feb 28 2016 18:31:21 GMT+0100 (CET)
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

#ifndef __N_173826070108225409_533114535__PARSER_TOOLS_HPP__
# define __N_173826070108225409_533114535__PARSER_TOOLS_HPP__

#include <utility>
#include <tools/ct_tuple.hpp>
#include <tools/genseq.hpp>
#include <tools/execute_pack.hpp>

#include "grammar_attributes.hpp"
#include "lexem_list.hpp"

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      /// \brief Manages lists of tuples
      /// The stack is stack-allocated, so no dynamic allocation here
      template<typename SyntaxClass, size_t MaxCount, typename TypeT, typename TypeList>
      class tuple_stack
      {
        public:
          constexpr tuple_stack() = default;

          /// \brief Push a new value to the stack
          template<typename T>
          constexpr void push(TypeT type, size_t state_index, T val)
          {
            stack[stack_size].template set<T>(val);
            type_stack[stack_size] = type;
            state_stack[stack_size] = state_index;
            ++stack_size;
          }
          /// \brief Get the top value
          template<typename T>
          constexpr T get(size_t index = 0)
          {
            return stack[stack_size - 1 - index].template get<T>();
          }

        private:
          template<typename Ret, typename... Args, size_t... Idxs>
          constexpr Ret _fwd_call(Ret (*function)(Args...), size_t initial, cr::seq<Idxs...>)
          {
            return function(stack[initial + Idxs].template get<Args>()...);
          }

          template<typename Ret, typename... Args>
          constexpr void sub_call_pop_push(Ret (*function)(Args...), size_t dest_elem)
          {
            stack[dest_elem].template set<Ret>(_fwd_call(function, dest_elem, cr::gen_seq<sizeof...(Args)>()));
          }

          template<template<e_forward_mode> class Type>
          constexpr void sub_call_pop_push(Type<e_forward_mode::direct>, size_t dest_elem)
          {
            using attr = Type<e_forward_mode::direct>;
            if (attr::index > 0)
              stack[dest_elem] = stack[dest_elem + attr::index];
          }

          template<template<e_forward_mode> class Type>
          constexpr void sub_call_pop_push(Type<e_forward_mode::token_value>, size_t dest_elem)
          {
            using attr = Type<e_forward_mode::direct>;
            using result_type = typename SyntaxClass::token_type::value_t;
            stack[dest_elem].template set<result_type>(stack[dest_elem + attr::index].template get<typename SyntaxClass::token_type>().value);
          }

        public:
          /// \brief Call function, pop the number number of argument, and push the return value
          /// \note Before using this, matches_production_rule must have returned true ! (no check are performed)
          /// \return the state to go
          template<size_t Count, typename Type>
          constexpr size_t call_pop_push(TypeT type, Type t)
          {
            const size_t dest_elem = stack_size - (Count);
            // it both call and push
            sub_call_pop_push(t, dest_elem);
            type_stack[dest_elem] = type;
            stack_size = dest_elem + 1;

            return state_stack[dest_elem];
          }

          /// \brief Return the size of the stack
          constexpr size_t size() const
          {
            return stack_size;
          }

          /// \brief Return the top type. The stack must not be empty !
          constexpr TypeT get_top_type() const
          {
            return type_stack[stack_size - 1];
          }

          /// \brief Return the top type. The stack must not be empty !
          constexpr TypeT get_type(size_t index = 0) const
          {
            return type_stack[stack_size - 1 - index];
          }

          /// \brief Check if a prod rule matches the stack
          template<typename FollowSet, typename... T>
          constexpr bool matches_production_rule(const lexem_list<SyntaxClass> &lookahead)
          {
            if (sizeof...(T) > stack_size)
              return false;

            bool res = true;
            size_t rev_index = stack_size - sizeof...(T);
            NEAM_EXECUTE_PACK(
              res &= (type_stack[rev_index++] == T::value)
            );
            if (!FollowSet::size && res)
              return lookahead.is_last();
            if (res)
              return follow_set<FollowSet>::matches(lookahead.get_token().type);
            return false;
          }

          template<typename T> struct production_rule_matcher {};
          template<typename... T> struct production_rule_matcher<ct::type_list<T...>>
          {
            template<typename FollowSet>
            constexpr static bool test(tuple_stack &s, const lexem_list<SyntaxClass> &lookahead)
            {
              return s.matches_production_rule<FollowSet, T...>(lookahead);
            }
          };

        private:
          template<typename FollowList, bool = false>
          struct follow_set
          {
            constexpr static bool matches(typename SyntaxClass::token_type::type_t lookahead)
            {
              if (FollowList::front::value == lookahead)
                return true;
              return follow_set<typename FollowList::pop_front>::matches(lookahead);
            }
          };

          template<bool X>
          struct follow_set<ct::type_list<>, X>
          {
            constexpr static bool matches(typename SyntaxClass::token_type::type_t)
            {
              return false;
            }
          };

        private:
          ct::tuple<TypeList> stack[MaxCount] = {};
          size_t state_stack[MaxCount] = {0};
          TypeT type_stack[MaxCount] = {0};
          size_t stack_size = 0;
      };

      /// \brief What actually "parses". It wraps the _state struct adding it the ability to consume a "stream" of token.
      template<typename SyntaxClass, typename State>
      struct parser_state
      {
        using uts_t = typename SyntaxClass::parser::uts_t;
        using type_t = typename SyntaxClass::token_type::type_t;

        /// \brief Extract ct::type_list<...> tpl argument pack
        template<typename List, bool = false>
        struct production_rule_matcher
        {
          constexpr static size_t test(uts_t &s, const lexem_list<SyntaxClass> &lookahead)
          {
            using rule = typename List::front;
            if (uts_t::template production_rule_matcher<typename rule::as_type_list>::template test<typename rule::follow_set>(s, lookahead))
              return s.template call_pop_push<rule::as_type_list::size>(rule::rule_name, rule::attribute::function);
            return production_rule_matcher<typename List::pop_front, false>::test(s, lookahead);
          }
        };

        template<bool X>
        struct production_rule_matcher<ct::type_list<>, X>
        {
          constexpr static size_t test(uts_t &, const lexem_list<SyntaxClass> &)
          {
            return -1;
          }
        };

        /// \brief Handle edges
        template<typename List, bool IsPost>
        struct on_edge
        {
          constexpr static size_t forward(uts_t &s, lexem_list<SyntaxClass> &ll, type_t type)
          {
            using current_edge = typename List::front;
            if (current_edge::name == type)
            {
              if (!IsPost)
              {
                const type_t type = ll.get_token().type;
                constexpr size_t state_index = SyntaxClass::parser::automaton_list::template get_type_index<State>::index;
                s.push(type, state_index, ll.get_token()); // in case of error, the last token is what caused the failure.
                // std::cout << " <- " << SyntaxClass::get_name_for_token_type(type) << " [" << ll.get_token().value << "] " << '\n'; // DEBUG
                ll = ll.get_next();
              }
              return parser_state<SyntaxClass, typename current_edge::state>::rec_parse(s, ll);
            }
            return on_edge<typename List::pop_front, IsPost>::forward(s, ll, type);
          }
        };
        template<bool IsPost>
        struct on_edge<ct::type_list<>, IsPost>
        {
          constexpr static size_t forward(uts_t &, lexem_list<SyntaxClass> &, type_t)
          {
            return -1;
          };
        };

        /// \brief The state entry point
        static constexpr size_t rec_parse(uts_t &stack, lexem_list<SyntaxClass> &ll)
        {
          constexpr size_t state_index = SyntaxClass::parser::automaton_list::template get_type_index<State>::index;

          // std::cout << "S" << state_index <<  " [" << State::final_rules::size; // DEBUG

          if (State::final_rules::size)
          {
            const size_t ret = production_rule_matcher<typename State::final_rules>::test(stack, ll);
            // std::cout << ", S" << long(ret) << "]x\n"; // DEBUG
            if (ret != size_t(-1))
              return ret;
          }
          // else std::cout << "]\n"; // DEBUG

          size_t forward_ret = on_edge<typename State::edges, false>::forward(stack, ll, ll.get_token().type);
          while (forward_ret == state_index)
          {
            forward_ret = on_edge<typename State::edges, true>::forward(stack, ll, stack.get_top_type());
            if (forward_ret == size_t(-1))
              forward_ret = on_edge<typename State::edges, false>::forward(stack, ll, ll.get_token().type);
          }

          // std::cout << "S" << state_index << "x->S" << long(forward_ret) << "\n"; // DEBUG
          return forward_ret;
        }
      };
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_173826070108225409_533114535__PARSER_TOOLS_HPP__*/