//
// file : grammar.hpp
// in : file:///home/tim/projects/alphyn/alphyn/grammar.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Tue Feb 23 2016 15:41:35 GMT+0100 (CET)
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

#ifndef __N_23904263722513032616_2623315966__GRAMMAR_HPP__
# define __N_23904263722513032616_2623315966__GRAMMAR_HPP__

#include <tools/ct_list.hpp>
#include <tools/embed.hpp>

#include "grammar_tools.hpp"
#include "grammar_attributes.hpp"

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      /// \brief A single production rule (must be included in a production_rule_set)
      /// The syntax class must be passed to every production rule
      template<typename SyntaxClass, typename Attribute, typename SyntaxClass::token_type::type_t... TokensOrRules>
      struct production_rule
      {
        production_rule() = delete;
        static_assert(sizeof...(TokensOrRules), "empty production_rule");

        using syntax_class = SyntaxClass;
        using type_t = typename SyntaxClass::token_type::type_t;

        /// \brief Return a type_list (merge_pack needs type lists)
        using as_type_list = ct::type_list<embed::embed<type_t, TokensOrRules>...>;

        using attribute = Attribute;

        // This will trigger if the attribute has a different arity than the number of Tokens Or Rules specified for that production
        static_assert((Attribute::arity <= 0 || Attribute::arity == sizeof...(TokensOrRules)), "number of parameter for the attribute is different from what can provide the production rule");

        // This will trigger if the attribute require at least x parameters but less are provided by the production
        static_assert((Attribute::arity >= 0 || (-Attribute::arity) <= sizeof...(TokensOrRules)), "attribute need to many parameters in production_rule");
      };

      /// \brief A set of production rule that shares the same "name"
      /// The syntax class must be passed to every production rule set :/
      template<typename SyntaxClass, typename SyntaxClass::token_type::type_t RuleName, typename... ProductionRules>
      struct production_rule_set
      {
        production_rule_set() = delete;
        static_assert(sizeof...(ProductionRules), "empty production_rule_set");

        using syntax_class = SyntaxClass;
        using type_t = typename SyntaxClass::token_type::type_t;

        static constexpr type_t rule_name = RuleName;

        /// \brief Return a type_list
        using as_type_list = ct::type_list<ProductionRules...>;

        template<typename X> struct forward_attr_ret_type { using type = typename X::attribute::return_type; };
        template<typename X> using is_void = std::is_same<void, X>;

        /// \brief The list of all possible return types
        using return_type_list = typename as_type_list::template for_each<forward_attr_ret_type>::template remove_if<is_void>::make_unique;
      };

      /// \brief The (parser) grammar
      /// The grammar class generate the LR(1) state-machine at compile-time
      template<typename SyntaxClass, typename SyntaxClass::token_type::type_t StartRule, typename... ProductionRuleSets>
      class grammar
      {
        public:
          grammar() = delete; // That is a total nonsens. A given grammar is just a type.
          static_assert(sizeof...(ProductionRuleSets), "empty grammar");

          using syntax_class = SyntaxClass;
          using type_t = typename SyntaxClass::token_type::type_t;

          static constexpr type_t start_rule = StartRule;


          using as_type_list = ct::type_list<ProductionRuleSets...>;
          using non_terminal_list = ct::type_list<embed::embed<type_t, ProductionRuleSets::rule_name>...>; // in order

          template<typename X> struct forward_return_type_list { using type = typename X::return_type_list; };

          /// \brief The list of all possible attribute return types
          using return_type_list = typename ct::merger
          <
            typename as_type_list::template for_each<forward_return_type_list>::flatten::make_unique,
            ct::type_list<typename syntax_class::token_type, type_t>
          >::type_list::make_unique;

        private: // check
          template<typename RS> struct is_start_rule { constexpr static bool value = (RS::rule_name == start_rule); };
          static constexpr long start_index = as_type_list::template find_if<is_start_rule>::index;

          // Well, alphyn could not find your start production_rule_set/production_rule in the grammar
          static_assert(start_index != -1, "Could not find the start production rule set in the grammar");
          using start_prod_rule_set = typename as_type_list::template get_type<start_index>;

          // Alphyn only support start production rule set than only have one rule
          static_assert(start_prod_rule_set::as_type_list::size == 1, "The start production set have more than one production rule");

        public:
          // type is production_rule<>
          using start_production_rule = typename start_prod_rule_set::as_type_list::template get_type<0>;
      };
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_23904263722513032616_2623315966__GRAMMAR_HPP__*/