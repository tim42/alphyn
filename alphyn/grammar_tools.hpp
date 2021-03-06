//
// file : grammar_tools.hpp
// in : file:///home/tim/projects/alphyn/alphyn/grammar_tools.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Tue Feb 23 2016 17:04:30 GMT+0100 (CET)
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

#ifndef __N_105957397248726540_1287622304__GRAMMAR_TOOLS_HPP__
# define __N_105957397248726540_1287622304__GRAMMAR_TOOLS_HPP__

#include <tools/ct_list.hpp> // for ct::type_list<>
#include <tools/merge_pack.hpp> // for ct::merger<>
#include <tools/embed.hpp> // for embed::embed<>

//                                               //
// Welcome to the land of functional programming //
//                                               //

//
// In this file are gathered all FIRST(..), FOLLOW(..), CLOSURE(..), GOTO(..)
// function for building a LR(1) automaton
// As it is written almost purely in compile-time functional C++ it may be, hum, quite difficult to read
// for someone that isn't used to this. That's the point of having all this stuff in one file.
// The code mostly rely on ct::type_list, located in <tools/ct_list.hpp>.
// Working with types allow to be certain that the generated code is correct
// (you can't do static_assert() on parameters of constexpr functions)
// Moreover, this "forces" the compiler to resolve everything at compile-time.
// It is also simpler to write it that way (partial specializations FTW) and the compiler is
// quite good dealing with types.
// I just hope that your IDE have some nice syntactic colors.
//
// This file is only focused on the grammar and transformation that can be applied to it.
// There's not functions in this file.
// If you look for something related to the parser or to some parsing, see parser_*.hpp files
//
namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      namespace internal
      {
        /// \brief value == true if T is a terminal, false if it's a non-terminal
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t T>
        struct _is_terminal
        {
          using type_t = typename SyntaxClass::token_type::type_t;

          static constexpr bool value = !ct::is_in_list<typename SyntaxClass::grammar::non_terminal_list, embed::embed<type_t, T>>::value;
        };
        /// \brief value == true if T is a non-terminal, false if it's a terminal
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t T>
        struct _is_non_terminal : public ct::is_in_list<typename SyntaxClass::grammar::non_terminal_list, embed::embed<typename SyntaxClass::token_type::type_t, T>>
        {
        };

        /// \brief Retrieve the set of terminals which can appear as the first elements of any chain of rules matching NonTerminal
        /// \note Stack is the stack of non-terminal being currently processed
        /// \note in the case you're being lost in that thing, the only property
        ///       you shall use is the last one: list (which is of the neam::ct::type_list<...> type)
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t NonTerminal, typename Stack = ct::type_list<>>
        struct _first
        {
          using type_t = typename SyntaxClass::token_type::type_t;

          // this will fail first if NonTerminal suddenly became a terminal (not a rule-name). _first MUST takes a Terminal
          // If you see that failure, uh. I don't know. Check your grammar ?
          // Oh. Your grammar is OK ?
          //              Good luck then :3
          static_assert(_is_non_terminal<SyntaxClass, NonTerminal>::value, "NonTerminal is not a non-terminal");


          static constexpr long index = SyntaxClass::grammar::non_terminal_list::template get_type_index<embed::embed<type_t, NonTerminal>>::index; // get the index
          using rule_set = typename SyntaxClass::grammar::as_type_list::template get_type<index>; // get the rule-set from the index

          // alphyn sanity check
          static_assert(rule_set::rule_name == NonTerminal, "something is fucked-up somewhere [sanity report#1 from alphyn/_first]");

          using rules = typename rule_set::as_type_list; // get the list of rules from the rule-set

          // a nice filter pattern used to recursively apply _first on all non-terminals and keep present terminals in the list
          template<typename...> struct _filter  { using type_list = ct::type_list<>; };

          template<bool IsNonTerminal, bool IsBeingProcessed, typename Current>
          struct _apply
          {
            // here we are a terminal (IsNonTerminal == false)
            // IsBeingProcessed doesn't matter for terminals
            using current_embed = typename Current::as_type_list::template get_type<0>;
            using list = ct::type_list<current_embed>;
          };

          template<typename Current, typename... Types>
          struct _filter<Current, Types...> :
          public ct::merger
          <
            typename _apply
            <
              _is_non_terminal<SyntaxClass, Current::as_type_list::template get_type<0>::value>::value,
              ct::is_in_list<Stack, typename Current::as_type_list::template get_type<0>>::value,
              Current
            >::list,
            typename _filter<Types...>::type_list
          >
          {
          };

          // and finally the result:
          using list = typename ct::extract_types<_filter, rules>::type::type_list::make_unique;
        };
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t NonTerminal, typename Stack>
        template<typename Current>
        struct _first<SyntaxClass, NonTerminal, Stack>::_apply<true, true, Current>
        {
          // here we are a non-terminal, but that non-terminal is already being processed
          using list = ct::type_list<>;
        };
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t NonTerminal, typename Stack>
        template<typename Current>
        struct _first<SyntaxClass, NonTerminal, Stack>::_apply<true, false, Current> :
        public _first // here we are a terminal
        <
          SyntaxClass,
          Current::as_type_list::template get_type<0>::value,
          typename Stack::template append<typename Current::as_type_list::template get_type<0>>
        >
        {
        };

        /// \brief Retrieve the set of terminals that can appear immediately after the non-terminal NonTerminal
        /// This is helpful to determine the lookahead terminal (token)
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t NonTerminal, typename Stack = ct::type_list<>>
        struct _follow
        {
          using type_t = typename SyntaxClass::token_type::type_t;

          // this will fail first if NonTerminal suddenly became a terminal (not a rule-name). _first MUST takes a Terminal
          // If you see that failure, uh. I don't know. Check your grammar ?
          // Oh. Your grammar is OK ?
          //              Good luck then :3
          static_assert(_is_non_terminal<SyntaxClass, NonTerminal>::value, "NonTerminal is not a non-terminal");


          static constexpr long index = SyntaxClass::grammar::non_terminal_list::template get_type_index<embed::embed<type_t, NonTerminal>>::index; // get the index
          using rule_set = typename SyntaxClass::grammar::as_type_list::template get_type<index>; // get the rule-set from the index

          // alphyn sanity check
          static_assert(rule_set::rule_name == NonTerminal, "something is fucked-up somewhere [sanity report#1 from alphyn/_follow]");

          static constexpr type_t rule_name = rule_set::rule_name; // get the name of the rule-set


          // Now I have to walk the entire grammar, and for each production rules, I have to find if somewhere there's "rule_name"
          // and then retrieve the element just next. If it's a terminal, push it to the list, if not, apply the _first<> to him and merge the result to the list.
          // If the rule_name non-terminal is at the end, I need to apply follow on the production_rule name and merge the list.

          // iterate over all production_rule_set
          template<typename...> struct _filter_rule_set { using type = ct::type_list<>; };
          // iterate over all production_rule of a production_rule_set
          template<type_t PRName, typename...> struct _filter_rule { using type = ct::type_list<>; };
          // perform an action on the current element
          template<bool IsNonTerminal, bool IsBeingProcessed, bool Follow, typename Elem>
          struct _apply_elem
          {
            // the default spec we are in the case where IsNonTerminal == false (we are a terminal)
            using type = ct::type_list<Elem>;
          };
          // perform an action on the current production_rule
          template<long Index, long LastElementIndex, typename PR, type_t PRName>
          struct _apply_rule
          {
            using next_elem = typename PR::as_type_list::template get_type<Index + 1>;
            // the default spec we are in the case where Index != LastElementIndex (element not the last, but found)
            // We want a _first on this element
            using type = typename _apply_elem<_is_non_terminal<SyntaxClass, next_elem::value>::value, ct::is_in_list<Stack, next_elem>::value, false, next_elem>::type;
          };

          // spec of _filter_rule_set
          template<typename Current, typename... Types>
          struct _filter_rule_set<Current, Types...>
          {
            template<typename... Rules> using partial_filter_rule = _filter_rule<Current::rule_name, Rules...>;
            using type = typename ct::merger
            <
              typename ct::extract_types<partial_filter_rule, typename Current::as_type_list>::type::type,
              typename _filter_rule_set<Types...>::type
            >::type_list;
          };
          // spec of _filter_rule
          template<type_t PRName, typename Current, typename... Types>
          struct _filter_rule<PRName, Current, Types...>
          {
            static constexpr long index = Current::as_type_list::template get_type_index<embed::embed<type_t, rule_name>>::index;
            using type = typename ct::merger
            <
              typename _apply_rule<index, long(Current::as_type_list::size) - 1, Current, PRName>::type,
              typename _filter_rule<PRName, Types...>::type
            >::type_list;
          };

          // The result of all this code:
          using list = typename ct::extract_types<_filter_rule_set, typename SyntaxClass::grammar::as_type_list>::type::type::make_unique;
        };
        // _follow's _apply_elem partial specializations
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t NonTerminal, typename Stack>
        template<typename Elem>
        struct _follow<SyntaxClass, NonTerminal, Stack>::_apply_elem<true, true, true, Elem>
        {
          // here we are a non-terminal, but that non-terminal is already being processed for _follow
          using type = ct::type_list<>;
        };
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t NonTerminal, typename Stack>
        template<typename Elem>
        struct _follow<SyntaxClass, NonTerminal, Stack>::_apply_elem<true, false, true, Elem>
        {
          // here we are a non-terminal, and that non-terminal is not being processed for _follow
          using type = typename _follow<SyntaxClass, Elem::value, typename ct::append_type<Elem, Stack>::type>::list;
        };
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t NonTerminal, typename Stack>
        template<bool IsBeingProcessed, typename Elem>
        struct _follow<SyntaxClass, NonTerminal, Stack>::_apply_elem<true, IsBeingProcessed, false, Elem>
        {
          // here we are a terminal, querying for _first<> (IsBeingProcessed doesn't matter here)
          using type = typename _first<SyntaxClass, Elem::value>::list;
        };

        // _follow's _apply_rule partial specializations
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t NonTerminal, typename Stack>
        template<long Index, typename PR, typename SyntaxClass::token_type::type_t PRName>
        struct _follow<SyntaxClass, NonTerminal, Stack>::_apply_rule<Index, Index, PR, PRName>
        {
          using next_elem = embed::embed<typename SyntaxClass::token_type::type_t, PRName>;
          // the default spec we are in the case where Index == LastElementIndex (element is the last)
          // We want a _follow on the rule
          using type = typename _follow<SyntaxClass, NonTerminal, Stack>::template _apply_elem
          <
            _is_non_terminal<SyntaxClass, next_elem::value>::value,
            ct::is_in_list<Stack, next_elem>::value,
            true,
            next_elem
          >::type;
        };
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t NonTerminal, typename Stack>
        template<long LastElementIndex, typename PR, typename SyntaxClass::token_type::type_t PRName>
        struct _follow<SyntaxClass, NonTerminal, Stack>::_apply_rule<-1, LastElementIndex, PR, PRName>
        {
          // The element is not found in this production_rule, we just skip that rule
          using type = ct::type_list<>;
        };

        /// \brief A transition (see _closure and prod_rule_wrapper)
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t Name, typename PRW>
        struct _transition
        {
          using syntax_class = SyntaxClass;
          using type_t = typename SyntaxClass::token_type::type_t;
          static constexpr type_t name = Name;
          using production_rule = PRW;
        };

        /// \brief Wrap production rules, add a position marker and a follow set (that default to the whole follow set of PRName, and that may be wrong for LR(1))
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t PRName, size_t Position, typename ProdRule, typename FollowSet = typename _follow<SyntaxClass, PRName>::list>
        struct prod_rule_wrapper
        {
          using syntax_class = SyntaxClass;
          using type_t = typename SyntaxClass::token_type::type_t;
          static constexpr type_t rule_name = PRName;
          using as_type_list = typename ProdRule::as_type_list;
          using attribute = typename ProdRule::attribute;
          static constexpr size_t position = Position;

          static_assert(Position <= as_type_list::size, "something is fucked-up somewhere [sanity report#1 from alphyn/prod_rule_wrapper]");

          using first_set = typename _first<SyntaxClass, PRName>::list;
          using follow_set = FollowSet; // LR(1)

          // Indicate whether or not the Position is a past-the-end position (we can't have a transition)
          static constexpr bool is_final = (position == as_type_list::size);

          template<size_t NewPosition, bool IsFinal>
          struct _switch { using type = ct::type_list<>; };
          template<size_t NewPosition> struct _switch<NewPosition, false>
          {
            using type = ct::type_list
            <
              _transition
              <
                SyntaxClass,
                as_type_list::template get_type<NewPosition - 1>::value,
                prod_rule_wrapper<SyntaxClass, PRName, NewPosition, ProdRule, FollowSet>
              >
            >;
          };

          // there, we list the possible transition (aka the next *terminal, if any). It has a ct::type_list<transition> or a ct::type_list<> type (if is_final == true).
          using transition = typename _switch<position + 1, is_final>::type;

          template<typename NewFollowSet>
          using set_follow_set = prod_rule_wrapper<SyntaxClass, PRName, Position, ProdRule, NewFollowSet>;
        };

        /// \brief Transform a production_rule_set into a ct::type_list of prod_rule_wrappers
        /// The result is in `::type`.
        template<typename PRS, size_t Position = 0, typename FollowSet = typename _follow<typename PRS::syntax_class, PRS::rule_name>::list>
        struct wrap_production_rule_set
        {
          template<typename... PR>
          using _wrapper = ct::type_list<prod_rule_wrapper<typename PR::syntax_class, PRS::rule_name, Position, PR, FollowSet>...>;

          // and the result:
          using type = typename ct::extract_types<_wrapper, typename PRS::as_type_list>::type;
        };
        /// \brief Same as wrap_production_rule_set, but for a non-terminal
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t NTName, size_t Position = 0, typename FollowSet = typename _follow<SyntaxClass, NTName>::list>
        struct wrap_non_terminal
        {
          template<typename RS> struct this_is_the_non_terminal_were_looking_for { constexpr static bool value = (RS::rule_name == NTName); };
          static constexpr long index = SyntaxClass::grammar::as_type_list::template find_if<this_is_the_non_terminal_were_looking_for>::index;

          // Alphyn could not find your non-terminal in the grammar (in the form `NTName -> ...` )
          static_assert(index != -1, "Could not find the a production rule that produce `NTName` in the grammar");
          using prod_rule_set = typename SyntaxClass::grammar::as_type_list::template get_type<index>;

          // and the result:
          using type = typename wrap_production_rule_set<prod_rule_set, Position, FollowSet>::type;
        };

        // for a terminal:
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t Name, bool IsNonTerminal>
        struct _first_or_name_switch
        {
          using list = ct::type_list<embed::embed<typename SyntaxClass::token_type::type_t, Name>>;
        };
          // for a non-terminal:
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t Name>
        struct _first_or_name_switch<SyntaxClass, Name, true> : public _first<SyntaxClass, Name>
        {
        };

        /// \brief _first if Name is a non-terminal, `type_list < embed::embed < type_t, Name>>` if Name is a terminal
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t Name>
        struct _first_or_name : public _first_or_name_switch<SyntaxClass, Name, _is_non_terminal<SyntaxClass, Name>::value>
        {
        };

        // merge lookahead sets
        template<typename X>
        using forward_lookahead = typename X::follow_set;

        /// \brief Compute the LR(1) closure for a given prod_rule_wrapper<> (as PRW)
        template<typename SyntaxClass, typename PRWList, typename LookAhead = typename PRWList::template direct_for_each<forward_lookahead>::flatten::make_unique, typename Stack = ct::type_list<>>
        struct _closure
        {
          using type_t = typename SyntaxClass::token_type::type_t;

          template<typename PRW, bool HasFollowing>
          struct _lookahead_switch : public _first_or_name<SyntaxClass, PRW::as_type_list::template get_type<PRW::position + 1>::value>
          {
            // we have a following element
          };
          template<typename PRW>
          struct _lookahead_switch<PRW, false>
          {
            // we don't have a following element
            using list = LookAhead;
          };
          template<typename PRW>
          using apply_lookahead = typename _lookahead_switch<PRW, (PRW::position < (PRW::as_type_list::size - 1))>::list;

          // get the lookahead list for subsequent application of _closure
          using ctx_look_ahead = typename PRWList::template direct_for_each<apply_lookahead>::flatten::make_unique;

          template<typename PRW, bool IsNonTerminal>
          struct _nonterminal_switch_2
          {
            // not a non-terminal
            using type = ct::type_list<>;
          };

          template<typename PRW>
          struct _nonterminal_switch_2<PRW, true>
          : public wrap_non_terminal<SyntaxClass, PRW::as_type_list::template get_type<PRW::position>::value, 0, ctx_look_ahead>
          {
            // this is a non-terminal
          };

          // Get the list of all non-terminals in the PRWList set
          template<typename PRW, bool IsPastTheEnd>
          struct _nonterminal_switch
          {
            // we are past the end or a non-terminal, nothing to do.
            using type = ct::type_list<>;
          };
          template<typename PRW>
          struct _nonterminal_switch<PRW, false> : public _nonterminal_switch_2
            <
              PRW,
              _is_non_terminal
              <
                SyntaxClass,
                PRW::as_type_list::template get_type<PRW::position>::value
              >::value
            >
          {
          };
          template<typename PRW>
          struct apply_nonterminal_switch : public _nonterminal_switch
            <
              PRW,
              (PRW::position > (PRW::as_type_list::size - 1))
            >
          {
          };

          // is in stack filter
          template<typename X> using stack_elem = ct::type_list<X/*, PRWList*/>;
          template<typename X> using is_in_stack = typename Stack::template is_in_list<stack_elem<X>>;

          // the list of non-terminals
          using nonterminal_list = typename PRWList::template for_each<apply_nonterminal_switch>::make_unique::template remove_if<is_in_stack>;
          // the new stack
          using new_stack = typename Stack::template append_list<typename nonterminal_list::template direct_for_each<stack_elem>>;

          template<typename PRW, bool HasFollowing>
          struct _sub_lookahead_switch : public _first_or_name<SyntaxClass, PRW::as_type_list::template get_type<PRW::position + 1>::value>
          {
            // we have a following element
          };
          template<typename PRW>
          struct _sub_lookahead_switch<PRW, false>
          {
            // we don't have a following element
            using list = ctx_look_ahead;
          };
          template<typename PRW>
          using apply_sub_lookahead = typename _sub_lookahead_switch<PRW, (PRW::position < (PRW::as_type_list::size - 1))>::list;

          template<typename List, bool IsEmtpy>
          struct sub_closure_result { using list = ct::type_list<>; };
          template<typename List>
          struct sub_closure_result<List, false> :
          public _closure<SyntaxClass, List, typename List::template direct_for_each<apply_sub_lookahead>::flatten::make_unique, new_stack>
          {
          };

          template<typename X>
          using apply_sub_closure = typename sub_closure_result<X, false>::list;

          using pre_list = typename nonterminal_list::template direct_for_each<apply_sub_closure>::flatten::template prepend_list<PRWList>;

          // test if two prw matches (excluding the follow_set)
          template<typename ToFind, typename X>
          struct is_same_prw
          {
            static constexpr bool value = (ToFind::rule_name == X::rule_name)
                                        && std::is_same<typename ToFind::as_type_list, typename X::as_type_list>::value;
          };

          // merge the follow sets
          template<typename X>
          struct prw_consolidate
          {
            template<typename Y> using _is_same_prw = is_same_prw<X, Y>;
            using new_follow_set = typename pre_list::template filter_by<_is_same_prw>::template direct_for_each<forward_lookahead>::flatten::make_unique;

            using type = typename X::template set_follow_set<new_follow_set>;
          };

          // the result
          using list = typename pre_list::template for_each<prw_consolidate>::make_unique;
        };

        /// \brief An edge in the automaton
        template<typename SyntaxClass, typename SyntaxClass::token_type::type_t Name, typename State>
        struct _edge
        {
          static constexpr typename SyntaxClass::token_type::type_t name = Name; ///< \brief The name of the transition (token type / ..)
          using state = State; ///< \brief The state linked by this edge
        };

        /// \brief An automaton's (quite raw) state.
        /// \note The compiler can choose to have a lazy evaluation of the graph, but as_type_list forces it to create the whole graph
        template<typename SyntaxClass, typename ClosureResultList>
        struct _state
        {
          using type_t = typename SyntaxClass::token_type::type_t;

          template<typename X>
          using transition_getter = typename X::transition;

          // the list of possible transitions
          using transition_list = typename ClosureResultList::template direct_for_each<transition_getter>::flatten::make_unique;

          // retrieve the name of a transition
          template<typename TR> using name_getter = embed::embed<typename TR::type_t, TR::name>;
          // retrieve the production_rule of a transition
          template<typename TR> using production_rule_getter = typename TR::production_rule;

          // filter a list of transition by a name
          template<type_t ToMatch, typename TR> struct name_filter { static constexpr bool value = (ToMatch == TR::name); };

          // create a list of all possible names (each name occurs at most one time)
          using transition_names = typename transition_list::template direct_for_each<name_getter>::make_unique;

          template<type_t ToMatch, typename List>
          struct get_name_list_filter
          {
            template<typename X> using filter = name_filter<ToMatch, X>;
            using type = typename List::template filter_by<filter>;
          };

          // make a transition_list suitable for applying _closure_list_nl on it:
          // gather transitions by their names (create lists per name)
          template<typename List, typename Transitions>
          struct _transition_list_builder :
          public _transition_list_builder
          <
            typename List::template append
            <
              _transition
              <
                SyntaxClass, Transitions::front::value,
                // the list of prod_rule_wrapper of all the transitions that matches the name CurrentName
                typename get_name_list_filter<Transitions::front::value, transition_list>::type::template direct_for_each<production_rule_getter>::make_unique
              >
            >,
            typename Transitions::pop_front
          >
          {
          };
          template<typename List>
          struct _transition_list_builder<List, ct::type_list<>>
          {
            using list = List;
          };

          using grouped_transition_list = typename _transition_list_builder<ct::type_list<>, transition_names>::list;

          /// \brief The maker of the edges
          template<typename List, typename... Transitions>
          struct _edges_maker // final
          {
            using list = typename List::make_unique;
          };
          template<typename List, typename CurrentTransition, typename... Transitions>
          struct _edges_maker<List, CurrentTransition, Transitions...>
          : public _edges_maker
          <
            typename ct::append_type
            <
              _edge
              <
                SyntaxClass, CurrentTransition::name,
                _state
                <
                  SyntaxClass,
                  typename _closure
                  <
                    SyntaxClass,
                    typename CurrentTransition::production_rule // list. In fact, this is a list. This is a misuse of _transition, but it works.
                  >::list
                >
              >,
              List
            >::type,
            Transitions...
          >
          {
          };

          template<typename... X>
          using edges_maker = _edges_maker<ct::type_list<>, X...>;
          using edges = typename ct::extract_types<edges_maker, grouped_transition_list>::type::list;

          template<typename PRW> struct is_a_final_rule { static constexpr bool value = PRW::is_final; };

          // the graph walker
          // About that, g++ is really capricious and a small little change can destroy everything.
          // The key point for this thing to work is to have them defined per state,
          // meaning that at least one of _graph_walker and _get_list should depend
          // somehow of the current state or any of the types it defines.
          // `typename S` is just an extra security, for being future proof and
          // `avoiding headaches finding the reason why the fucking ::list wasn't
          // a property of _get_list<>.
          // I can't get clang to work with this.
          template<typename S, typename List, typename... Edges> struct _graph_walker : public List {}; // final

          template<typename S, typename List, typename Current, typename... Edges>
          struct _graph_walker<S, List, Current, Edges...> : public
          _graph_walker
          <
            _state,
            typename Current::state::template get_list<List>,
            Edges...
          >
          {
          };

          template<typename List, bool IsInList>
          struct _get_list
          {
            using list = List;
          };

          template<typename List>
          struct _get_list<List, false> 
          {
            using new_list = typename List::template append<_state>;

            template<typename... X>
            using initial_graph_walker = _graph_walker<_state, new_list, X...>;
            using list = typename ct::extract_types<initial_graph_walker, edges>::type;
          };

          template<typename List>
          using get_list = typename _get_list<List, ct::is_in_list<List, _state>::value>::list;

          // // public interface below: // //

          // the edges to other states (a ct::type_list of type _edge<>)
          // using edges = ...;

          // the (wrapped/transformed) productions rules of the current state
          // type is ct::type_list of prod_rule_wrapper
          using state_rules = ClosureResultList;

          // final rules (rules that doesn't have a possible transition)
          // type is ct::type_list of prod_rule_wrapper
          using final_rules = typename ClosureResultList::template filter_by<is_a_final_rule>;

          // non-final rules (rules that does have a possible transition)
          // type is ct::type_list of prod_rule_wrapper
          using non_final_rules = typename ClosureResultList::template remove_if<is_a_final_rule>;

          // export the graph as a ct::type_list<...>
          using as_type_list = get_list<ct::type_list<>>;
        };
      } // namespace internal

      /// \brief You may not use this class directly, but if we have to sort things in this file in the order of you should not use them directly, this class comes last
      /// Its only purpose is to be an "interface" to "easily" manipulate the grammar type of SyntaxClass
      /// I want it to be fully compile-time, so there's mostly usings that forward to some other constructs
      ///
      /// It has: terminal / non-terminal checking, first and follow, closure, get_transition_list and lr1_automaton
      template<typename SyntaxClass>
      struct grammar_tools
      {
        grammar_tools() = delete;

        using token_type = typename SyntaxClass::token_type;
        using type_t = typename token_type::type_t;

        /// \brief Return the list of terminals which can appear as the first elements of any chain of rules matching NT (which is a non-terminal)
        template<type_t NT>
        using first = typename internal::_first<SyntaxClass, NT>::list;

        /// \brief Retrieve the set of terminals that can appear immediately after the non-terminal NT
        template<type_t NT>
        using follow = typename internal::_follow<SyntaxClass, NT>::list;

        /// \brief Compute the LR(1) closure for a given prod_rule_wrapper<> (as PRW)
        template<type_t NT>
        using closure = typename internal::_closure<SyntaxClass, typename internal::wrap_non_terminal<SyntaxClass, NT>::type>::list;

        /// \brief Compute the LR(1) closure for a given prod_rule_wrapper<> (as PRW)
        /// \note you'll have to define a look-ahead (a good choice would be `follow< NT >` or `ct::type_list<>` whichever is the best for you)
        template<type_t NT, typename LookAhead>
        using closure_la = typename internal::_closure<SyntaxClass, typename internal::wrap_non_terminal<SyntaxClass, NT>::type, LookAhead>::list;

        /// \brief A type that represents the LR(1) automaton (/graph)
        /// It starts from the initial non-terminal (as defined by the grammar)
        using lr1_automaton = internal::_state<SyntaxClass, closure_la<SyntaxClass::grammar::start_rule, ct::type_list<>>>;

        /// \brief Check if T is a terminal (token-type) or a non-terminal (production rule)
        template<type_t T> using is_terminal = internal::_is_terminal<SyntaxClass, T>;
        /// \brief Check if T is a terminal (token-type) or a non-terminal (production rule)
        template<type_t NT> using is_non_terminal = internal::_is_non_terminal<SyntaxClass, NT>;
      };
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_105957397248726540_1287622304__GRAMMAR_TOOLS_HPP__*/