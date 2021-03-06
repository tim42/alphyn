//
// file : ct_parser.hpp
// in : file:///home/tim/projects/alphyn/alphyn/ct_parser.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Sat Mar 05 2016 12:05:27 GMT+0100 (CET)
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

#ifndef __N_31967108482145814827_52029795__CT_PARSER_HPP__
# define __N_31967108482145814827_52029795__CT_PARSER_HPP__

#include <tools/ct_list.hpp>
#include <tools/embed.hpp>
#include <tools/macro.hpp>
#include "grammar_attributes.hpp"

// this file contains almost the same thing than parser_tools.hpp (maybe I should rename parser_tools to cr_parser)
// but using meta-functions.

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      namespace internal
      {
        /// \brief An entry in the stack (a stack is simply a ct::type_list<>)
        template<typename TypeT, TypeT Type, typename ValueT, typename CTState>
        struct ct_stack_entry
        {
          static constexpr TypeT type = Type;
          using ct_state = CTState;
          using value = ValueT;

          template<TypeT NewType, typename NewCTState>
          using change_type_and_state = ct_stack_entry<TypeT, NewType, ValueT, NewCTState>;
        };

        // Perform the reduction operation (accounting the attribute)
        template<typename Rule, typename Stack, typename Attribute, size_t StackSzDiff, typename BaseStack>
        struct ct_apply_attribute
        {
          using substack = typename Stack::template sublist<0, StackSzDiff>;
          using ct_state = typename substack::back::ct_state;

          // extract the value for some wrapper types (tokens & embeds):
          // this help keeping some compatibility with the "'value' parser"
          template<typename Type>
          struct get_value { static constexpr Type value = Type(); };
          template<typename Type, Type Value>
          struct get_value<embed::embed<Type, Value>> { static constexpr Type value = Value; };
          template<typename SyntaxClass, const char *Str, long InitialIndex, bool Empty>
          struct get_value<typed_lexem_list<SyntaxClass, Str, InitialIndex, Empty>>
          {
            using tll = typed_lexem_list<SyntaxClass, Str, InitialIndex, Empty>;
            static constexpr typename tll::token_type value = tll::token;
          };
          // call the function
          template<typename... Types>
          struct call_function
          {
            using return_type = embed::embed<typename Attribute::return_type, Attribute::function(get_value<typename Types::value>::value...)>;
          };

          // the default is to have Attribute as a function
          using result = typename ct::extract_types<call_function, typename substack::reverse>::type::return_type;

          // the result
          using result_stack = typename BaseStack::template prepend<ct_stack_entry<typename Rule::type_t, Rule::rule_name, result, ct_state>>;
        };

        template
        <
          typename Rule,
          typename Stack,
          template<typename FunctionType, FunctionType Function, typename...> class Attribute,
          size_t StackSzDiff,
          typename BaseStack,
          typename IndexEmbed
        >
        struct ct_apply_attribute<Rule, Stack, Attribute<e_special_attributes, e_special_attributes::forward, IndexEmbed>, StackSzDiff, BaseStack>
        {
          using substack = typename Stack::template sublist<0, StackSzDiff>;
          using ct_state = typename substack::back::ct_state;
          using elem = typename substack::template get_type<substack::size - 1 - IndexEmbed::value>;

          // the result
          using result_stack = typename BaseStack::template prepend<typename elem::template change_type_and_state<Rule::rule_name, ct_state>>;
        };
        template
        <
          typename Rule,
          typename Stack,
          template<typename FunctionType, FunctionType Function, typename...> class Attribute,
          size_t StackSzDiff,
          typename BaseStack,
          typename IndexEmbed
        >
        struct ct_apply_attribute<Rule, Stack, Attribute<e_special_attributes, e_special_attributes::value_forward, IndexEmbed>, StackSzDiff, BaseStack>
        {
          using substack = typename Stack::template sublist<0, StackSzDiff>;
          using ct_state = typename substack::back::ct_state;
          using lexem = typename substack::template get_type<substack::size - 1 - IndexEmbed::value>::value;
          using result = embed::embed<typename lexem::token_type::value_t, lexem::token.value>;

          // the result, wrapped in embed::embed<>
          using result_stack = typename BaseStack::template prepend<ct_stack_entry<typename Rule::type_t, Rule::rule_name, result, ct_state>>;
        };
        template
        <
          typename Rule,
          typename Stack,
          template<typename FunctionType, FunctionType Function, typename...> class Attribute,
          size_t StackSzDiff,
          typename BaseStack,
          typename SynthesizerWrapper
        >
        struct ct_apply_attribute<Rule, Stack, Attribute<e_special_attributes, e_special_attributes::synthesizer, SynthesizerWrapper>, StackSzDiff, BaseStack>
        {
          using substack = typename Stack::template sublist<0, StackSzDiff>;
          using ct_state = typename substack::back::ct_state;
          // unwrap the synthesizer
          template<typename... X> using synthesizer = typename SynthesizerWrapper::template synthesizer<typename X::value...>;
          using result = typename ct::extract_types<synthesizer, typename substack::reverse>::type::type;

          // the result
          using result_stack = typename BaseStack::template prepend<ct_stack_entry<typename Rule::type_t, Rule::rule_name, result, ct_state>>;
        };

        /// \brief A matcher for a stack and a production rule
        template<typename Stack, typename Rule, typename LookAheadLL>
        struct ct_stack_production_rule_matcher
        {
          using prod_rule_list = typename Rule::as_type_list;
          using lookahead_list = typename Rule::follow_set;
          using type_t = typename LookAheadLL::token_type::type_t;

          template<typename CStack, typename PRL, bool GoneWrong, bool IsLookAheadOK>
          struct ctsprm_iter : public ctsprm_iter<typename CStack::pop_front, typename PRL::pop_back, (CStack::front::type != PRL::back::value), IsLookAheadOK> {};

          template<typename CStack>
          struct ctsprm_iter<CStack, ct::type_list<>, false, true> // The end (we call the function / forward / synthesize / ...)
          {
            using res = ct_apply_attribute<Rule, Stack, typename Rule::attribute, prod_rule_list::size, CStack>;

            constexpr static bool is_ok = true;
            using new_stack = typename res::result_stack;
            using dest_state = typename res::ct_state;
          };
          struct bad_end // when the lookahead is not in the list, when that does not matches, when the stack is shorter than the prl, ...
          {
            constexpr static bool is_ok = false;
            using dest_state = void;
            using new_stack = Stack;
          };
          template<typename CStack, typename PRL> struct ctsprm_iter<CStack, PRL, true, true> : public bad_end {}; // Doesn't match (early exit / could be the end)
          template<typename CStack, typename PRL> struct ctsprm_iter<CStack, PRL, false, false> : public bad_end {}; // Doesn't match (early exit / could be the end)
          template<typename PRL> struct ctsprm_iter<ct::type_list<>, PRL, false, false> : public bad_end {}; // The end of the stack
          template<typename PRL> struct ctsprm_iter<ct::type_list<>, PRL, true, false> : public bad_end {}; // The end of the stack
          template<typename PRLItem1, typename... PRLNext>
          struct ctsprm_iter<ct::type_list<>, ct::type_list<PRLItem1, PRLNext...>, false, true>
            : public bad_end {}; // The end of the stack (also make sure the prl is not empty too)
          template<typename PRL> struct ctsprm_iter<ct::type_list<>, PRL, true, true> : public bad_end {}; // The end of the stack

          template<typename LAList, typename LA>
          struct lookahead_matcher : public LAList::template is_in_list< embed::embed<type_t, LA::token.type> > {};

          template<typename LA>
          struct lookahead_matcher<ct::type_list<>, LA>
          {
            static constexpr bool value = LA::is_last;
          };

          using res = ctsprm_iter<Stack, prod_rule_list, false, lookahead_matcher<lookahead_list, LookAheadLL>::value>;

          // the results
          constexpr static bool value = res::is_ok;
          using new_stack = typename res::new_stack;
          using dest_state = typename res::dest_state;
        };

        /// \brief the "type" ct parser
        /// \note It's mostly the meta-function equivalent of the parser_state class
        /// \p TLL is the typed lexem list (see the lexer class)
        /// \p Stack is the stack (the stack is a bt special as we prepend, not append)
        template<typename SyntaxClass, typename State, typename TLL, typename Stack>
        struct ct_parser_rec
        {
          using state = State;
          using type_t = typename SyntaxClass::token_type::type_t;
          using token_type = typename SyntaxClass::token_type;

          // Lookup productions and match them with the Stack
          template<typename List, typename Result = void, bool IsAlreadyDone = false, typename = void>
          struct production_rule_matcher : public production_rule_matcher
          <
            typename List::pop_front,
            ct_stack_production_rule_matcher<Stack, typename List::front, TLL>,
            ct_stack_production_rule_matcher<Stack, typename List::front, TLL>::value
          >
          {
          };

          struct bad_end
          {
            static constexpr bool has_matched = false;
            using dest_state = void;
            using result = void;
          };

          // We are at the end
          template<bool IsAlreadyDone, typename X>
          struct production_rule_matcher<ct::type_list<>, void, IsAlreadyDone, X> : bad_end {}; // end without matches
          template<typename Result, typename X>
          struct production_rule_matcher<ct::type_list<>, Result, true, X> // lucky end
          {
            static constexpr bool has_matched = true;
            using dest_state = typename Result::dest_state;
            using result = Result;
          };
          template<typename Result, typename X>
          struct production_rule_matcher<ct::type_list<>, Result, false, X> : bad_end {}; // bad end
          template<typename X>
          struct production_rule_matcher<ct::type_list<>, void, false, X> : bad_end {}; // empty start

          template<typename List, typename Result, typename X>
          struct production_rule_matcher<List, Result, true, X> // early exit
          {
            static constexpr bool has_matched = true;
            using dest_state = typename Result::dest_state;
            using result = Result;
          };

          template<typename CTLL, typename CSTack, type_t ToMatch, typename List, typename OList, bool IsPost, bool Recurse = true>
          struct recurse_switcher :
          public ct_parser_rec
          <
            SyntaxClass, typename List::front::state,
            typename std::conditional<!IsPost, typename CTLL::next, CTLL>::type,
            typename std::conditional<!IsPost, typename CSTack::template prepend<ct_stack_entry<type_t, CTLL::token.type, CTLL, State>>, CSTack>::type
          >
          {
          };


          template<typename CTLL, typename CSTack, type_t ToMatch, typename List, typename OList, bool IsPost = false>
          struct on_edge : public recurse_switcher
          <
            CTLL, CSTack, ToMatch, List, OList, IsPost,
            (ToMatch == List::front::name)
          >
          {
          };
          template<typename CTLL, typename CSTack, type_t ToMatch, typename List, typename OList, bool IsPost>
          struct recurse_switcher<CTLL, CSTack, ToMatch, List, OList, IsPost, false> : public on_edge<CTLL, CSTack, ToMatch, typename List::pop_front, OList, IsPost>
          {
          };
          template<typename CTLL, typename CSTack, type_t ToMatch, typename OList>
          struct on_edge<CTLL, CSTack, ToMatch, ct::type_list<>, OList, false> // end, end of the end, failed
          {
            using stack = CSTack;
            using tll = CTLL;
            using dest_state = void;
          };
          template<typename CTLL, typename CSTack, type_t ToMatch, typename OList>
          struct on_edge<CTLL, CSTack, ToMatch, ct::type_list<>, OList, true> 
          : public on_edge<CTLL, CSTack, CTLL::token.type, OList, OList, false> // end, try with IsPost = false
          {
          };

          // handle the recursion, ...
          template<typename PRMResult, bool HasPRMMatched = false>
          struct switcher
          {
            // recursion when a reduction has not been done
            using initial_rec = on_edge<TLL, Stack, TLL::token.type, typename State::edges, typename State::edges, false>;

            template<bool Continue, typename Last = initial_rec>
            struct iterate : public
            iterate
            <
              std::is_same
              <
                typename on_edge<typename Last::tll, typename Last::stack, Last::stack::front::type, typename State::edges, typename State::edges, true>::dest_state,
                State
              >::value,
              on_edge<typename Last::tll, typename Last::stack, Last::stack::front::type, typename State::edges, typename State::edges, true>
             >
            {
//               using current = on_edge<typename Last::tll, typename Last::stack, Last::stack::front::type, typename State::edges, typename State::edges, true>;
//               using res = typename iterate<std::is_same<typename current::dest_state, State>::value, current>::res;
            };
            template<typename Last>
            struct iterate<false, Last> : public Last
            {
//               using res = Last;
            };

            using res = iterate<std::is_same<typename initial_rec::dest_state, State>::value, initial_rec>;

            using stack = typename res::stack;
            using tll = typename res::tll;
            using dest_state = typename res::dest_state;
          };
          template<typename PRMResult>
          struct switcher<PRMResult, true>
          {
            // results:
            using dest_state = typename PRMResult::dest_state;
            using stack = typename PRMResult::result::new_stack;
            using tll = TLL;
          };

          // we store here the result of the production rule matcher
          using prm = production_rule_matcher<typename State::final_rules>;

          // the switcher (stop after reducing the production rule or continue and recurse)
          using switcher_res = switcher<prm, prm::has_matched>;

          // final
          using stack = typename switcher_res::stack; // the stack at the end of the state
          using tll = typename switcher_res::tll;   // the token list at the end of the state
          using dest_state = typename switcher_res::dest_state; // internal: the destination state
        };
      } // namespace internal
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_31967108482145814827_52029795__CT_PARSER_HPP__*/