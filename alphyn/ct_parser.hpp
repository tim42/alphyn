//
// file : ct_parser.hpp
// in : file:///home/tim/projects/alphyn/alphyn/ct_parser.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Sat Mar 05 2016 12:05:27 GMT+0100 (CET)
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

#ifndef __N_31967108482145814827_52029795__CT_PARSER_HPP__
# define __N_31967108482145814827_52029795__CT_PARSER_HPP__

#include <tools/ct_list.hpp>
#include <tools/embed.hpp>
#include "grammar_attributes.hpp"

// this file contains almost the same thing than parser_tools.hpp (maybe I should rename parser_tools to cr_parser)
// but using meta-functions.

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      /// \brief An entry in the stack (a stack is simply a ct::type_list<>)
      template<typename TypeT, TypeT Type, typename ValueT, typename CTState>
      struct ct_stack_entry
      {
        static constexpr TypeT type = Type;
        using ct_state = CTState; // you may want to use ct_state::partial_parser_rec<TLL, STACK>
        using value = ValueT;
      };

      /// \p BaseStack is the stack without any element related to the attribute
      template<typename Stack, typename Attribute, size_t StackSzDiff, typename BaseStack>
      struct ct_apply_attribute
      {
        using substack = typename Stack::template sublist<0, StackSzDiff>;

        // extract the value for some wrapper types (tokens & embeds)
        template<typename Type>
        struct get_value { static constexpr Type value = Type(); };
        template<typename Type, Type Value>
        struct get_value<embed::embed<Type, Value>> { static constexpr Type value = Value; };
        template<typename SyntaxClass, const char *Str, long InitialIndex, bool Empty>
        struct get_value<typed_lexem_list<SyntaxClass, Str, InitialIndex, Empty>>
        {
          using tll = typed_lexem_list<SyntaxClass, Str, InitialIndex, Empty>;
          static constexpr typename tll::token_type::value_t value = tll::token.value;
        };
        // call the function
        template<typename... Types>
        struct call_function
        {
          using return_type = embed::embed<typename Attribute::return_type, Attribute::function(get_value<Types>::value...)>;
        };

        // the default is to have Attribute as a function
        using result = typename ct::extract_types<call_function, substack>::type::return_type;

        // the result
        using result_stack = typename BaseStack::template preprend<result>;
      };

      template
      <
        typename Stack,
        template<typename FunctionType, FunctionType Function, typename...> class Attribute,
        size_t StackSzDiff,
        typename BaseStack,
        typename IndexEmbed
      >
      struct ct_apply_attribute<Stack, Attribute<e_special_attributes, e_special_attributes::forward, IndexEmbed>, StackSzDiff, BaseStack>
      {
        using substack = typename Stack::template sublist<0, StackSzDiff>;

        // the result
        using result_stack = typename BaseStack::template prepend<typename substack::template get_type<IndexEmbed::value>>;
      };
      template
      <
        typename Stack,
        template<typename FunctionType, FunctionType Function, typename...> class Attribute,
        size_t StackSzDiff,
        typename BaseStack,
        typename IndexEmbed
      >
      struct ct_apply_attribute<Stack, Attribute<e_special_attributes, e_special_attributes::value_forward, IndexEmbed>, StackSzDiff, BaseStack>
      {
        using substack = typename Stack::template sublist<0, StackSzDiff>;
        using lexem = typename substack::template get_type<IndexEmbed::value>;

        // the result, wrapped in embed::embed<>
        using result_stack = typename BaseStack::template prepend<embed::embed<typename lexem::token_type::value_t, lexem::token.value>>;
      };
      template
      <
        typename Stack,
        template<typename FunctionType, FunctionType Function, typename...> class Attribute,
        size_t StackSzDiff,
        typename BaseStack,
        typename SynthesizerWrapper
      >
      struct ct_apply_attribute<Stack, Attribute<e_special_attributes, e_special_attributes::synthesizer, SynthesizerWrapper>, StackSzDiff, BaseStack>
      {
        using substack = typename Stack::template sublist<0, StackSzDiff>;
        // unwrap the synthesizer
        template<typename... X> using synthesizer = typename SynthesizerWrapper::template synthesizer<X...>;

        // the result
        using result_stack = typename BaseStack::template prepend<typename ct::extract_types<synthesizer, substack>::type::type>;
      };

      /// \brief A matcher for a stack and a production rule
      template<typename Stack, typename Rule, typename LookAheadLL>
      struct ct_stack_production_rule_matcher
      {
        using prod_rule_list = typename Rule::as_type_list;
        using lookahead_list = typename Rule::follow_set;
        using type_t = typename LookAheadLL::token_type::type_t;

        template<typename CStack, typename PRL, bool IsLookAheadOK, bool GoneWrong = false>
        struct ctsprm_iter // what actually check
        {
          using current_stack = typename CStack::front;
          using current_item = typename PRL::front;

          constexpr static bool current_is_ok = (current_stack::type == current_item::value);
          using res = ctsprm_iter<typename CStack::pop_front, typename PRL::popfront, current_is_ok>;
          // results
          constexpr static bool is_ok = res::is_ok;
          using new_stack = typename res::new_stack;
          using dest_state = typename res::dest_state;
        };
        template<typename CStack>
        struct ctsprm_iter<CStack, ct::type_list<>, false, true> // The end (we call the function / forward / synthesize / ...)
        {
          using base_stack = typename CStack::pop_front;
          using dest_state = typename CStack::front::ct_state;
          constexpr static size_t stack_diff = (Stack::size - base_stack::size);
          constexpr static bool is_ok = true;
          using new_stack = typename ct_apply_attribute<Stack, typename Rule::attribute, stack_diff, base_stack>::result_stack;
        };
        template<typename CStack, typename PRL, bool IsLookAheadOK>
        struct ctsprm_iter<CStack, PRL, true, IsLookAheadOK> // Doesn't match (early exit / could be the end)
        {
          constexpr static bool is_ok = false;
          using dest_state = void;
          using new_stack = Stack;
        };
        template<typename CStack, typename PRL, bool GoneWrong>
        struct ctsprm_iter<CStack, PRL, GoneWrong, false> // Lookahead doesn't match (early exit)
        {
          constexpr static bool is_ok = false;
          using dest_state = void;
          using new_stack = Stack;
        };
        template<typename PRL, bool GoneWrong, bool IsLookAheadOK>
        struct ctsprm_iter<ct::type_list<>, PRL, GoneWrong, IsLookAheadOK> // The end of the stack
        {
          constexpr static bool is_ok = false;
          using dest_state = void;
          using new_stack = Stack;
        };

        using res = ctsprm_iter<Stack, prod_rule_list, lookahead_list::template is_in_list<embed::embed<type_t, LookAheadLL::token::type>>::value>;

        // the results
        constexpr static bool value = res::is_ok;
        using new_stack = typename res::new_stack;
        using dest_state = typename res::dest_state;
      };

      /// \brief the "fully" ct parser
      /// \p TLL is the typed lexem list (see the lexer class)
      /// \p Stack is the stack (the stack is a bt special as we prepend and not append)
      template<typename SyntaxClass, typename State, typename TLL, typename Stack>
      struct ct_parser_rec
      {
        // used to recurse
        template<typename _TLL, typename _Stack>
        using partial_parser_rec = ct_parser_rec<SyntaxClass, State, _TLL, _Stack>;

        using type_t = typename SyntaxClass::token_type::type_t;
        using token_type = typename SyntaxClass::token_type;

        // Lookup productions and match them with the Stack
        template<typename List, typename Result = void, bool IsAlreadyDone = false>
        struct production_rule_matcher
        {
          using current = typename List::front;
          using remaining = typename List::pop_front;

          using matcher_result = ct_stack_production_rule_matcher<Stack, current, TLL>;
          constexpr static bool current_matches = matcher_result::value;
          using next = production_rule_matcher<remaining, matcher_result, current_matches>;
          constexpr static bool has_matched = next::has_matched;
          using result = typename next::result;
        };

        // We are at the end
        template<typename Result, bool IsAlreadyDone>
        struct production_rule_matcher<ct::type_list<>, Result, IsAlreadyDone>
        {
          static constexpr bool has_matched = IsAlreadyDone;
          using dest_state = ct_parser_rec;
          using result = Result;
        };
        // early exit of the loop
        template<typename List, typename Result>
        struct production_rule_matcher<List, Result, true>
        {
          static constexpr bool has_matched = true;
          using dest_state = ct_parser_rec;
          using result = Result;
        };

        template<typename CTLL, typename CSTack, type_t ToMatch, typename List, typename OList, bool IsPost = false>
        struct on_edge
        {
          using current = typename List::front;

          template<bool Recurse = false, typename ParserRec = void, typename = void>
          struct recurse_switcher
          {
            using next_list = typename List::pop_front;
            using res = on_edge<CTLL, CSTack, ToMatch, next_list, OList, IsPost>;
            static constexpr bool success = res::success;
          };

          template<typename ParserRec, typename X>
          struct recurse_switcher<true, ParserRec, X>
          {
            using res = ParserRec;// ct_parser_rec<SyntaxClass, typename current::state, rec_tll, rec_stack>;
            static constexpr bool success = true;
          };

          using rec_tll = typename std::conditional<IsPost, typename CTLL::next, CTLL>::type;
          using rec_stack = typename std::conditional<IsPost, typename CSTack::template prepend<CTLL>, CSTack>::type;

          template<typename X>
          struct recurse_switcher<true, ct_parser_rec, X>
          {
            struct res
            {
              using stack = rec_stack;
              using tll = rec_tll;
            };
            static constexpr bool success = true;
          };

          using rec_res = recurse_switcher<ToMatch == current::name, ct_parser_rec<SyntaxClass, typename current::state, rec_tll, rec_stack>>;

          // results
          using stack = typename rec_res::res::stack;
          using tll = typename rec_res::res::tll;
          static constexpr bool success = rec_res::success;
        };
        template<typename CTLL, typename CSTack, type_t ToMatch, typename OList>
        struct on_edge<CTLL, CSTack, ToMatch, ct::type_list<>, OList, false> // end, end of the end
        {
          using stack = CSTack;
          using tll = CTLL;
          static constexpr bool success = false;
        };
        template<typename CTLL, typename CSTack, type_t ToMatch, typename OList>
        struct on_edge<CTLL, CSTack, ToMatch, ct::type_list<>, OList, true> // end, try with IsPost = false
        {
          using res = on_edge<CTLL, CSTack, CTLL::token.type, OList, OList, false>;
          using stack = typename res::stack;
          using tll = typename res::tll;
          static constexpr bool success = res::success;
        };

        // handle the recursion, ...
        template<typename PRMResult, bool HasPRMMatched = false>
        struct switcher
        {
          // recursion when a reduction has not been done
          using initial_rec = on_edge<TLL, Stack, TLL::token.type, typename State::edges, typename State::edges, false>;

          template<bool Continue, typename Last = initial_rec>
          struct iterate
          {
            using current = on_edge<typename Last::tll, typename Last::stack, Last::stack::front::type, typename State::edges, typename State::edges, true>;
            using res = typename iterate<current::success, current>::res;
          };
          template<typename Last>
          struct iterate<false, Last>
          {
            using res = Last;
          };

          using res = typename iterate<!initial_rec::success, initial_rec>::res;

          using stack = typename res::stack;
          using tll = typename res::tll;
        };
        template<typename PRMResult>
        struct switcher<PRMResult, true>
        {
          // recursion when a reduction has been done
          using rec_state = typename PRMResult::dest_state::template partial_parser_rec<TLL, typename PRMResult::new_stack>;

          // results:
          using stack = typename rec_state::stack;
          using tll = typename rec_state::tll;
        };

        // we store here the result of the production rule matcher
        using prm = production_rule_matcher<typename State::final_rules>;

        // the switcher (stop after reducing the production rule or continue and recurse)
        using switcher_res = switcher<prm, prm::has_matched>;

        // final
        using stack = typename switcher_res::stack; // the stack at the end of the state
        using tll = typename switcher_res::tll;   // the token list at the end of the state
      };
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_31967108482145814827_52029795__CT_PARSER_HPP__*/