//
// file : bnf.hpp
// in : file:///home/tim/projects/alphyn/samples/bnf-meta-parser/bnf.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Mon Mar 14 2016 15:24:35 GMT+0100 (CET)
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

#ifndef __N_643712118211766787_1135227579__BNF_HPP__
# define __N_643712118211766787_1135227579__BNF_HPP__

#include <string> // for the generated get_name_for_token_type()

#include "alphyn.hpp"
#include "default_token.hpp"

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      namespace bnf
      {
        template<const char *TypeName, typename Type>
        struct attribute_db_entry
        {
          using type = Type;

          // a small comparison function
          static constexpr bool strcmp(const char *s, size_t start, size_t end)
          {
            size_t i = start;
            size_t j = 0;
            for (; i < end && TypeName[j] != '\0' && TypeName[j] == s[i]; ++i, ++j);
            if (i != end || TypeName[j] != '\0')
              return false;
            return true;
          }
        };

        template<typename... Entries>
        using attribute_db = ct::type_list<Entries...>;

        using token_type = neam::ct::alphyn::token<int>;

        /// \brief A BNF parser that generates lexer and parser for the input BNF grammar
        /// \param TypeDB should be a attribute_db< attribute_db_entry< string, attribute >, ... >
        /// \code bnf_meta_parser<AttributeDB>::generate_parser<BNFString> \endcode will generate the parser for the BNF grammar in BNFString
        /// \see generate_parser
        template<typename AttributeDB>
        struct bnf_meta_parser
        {
          // token relative things (type and invalid)
          using token_type = ct::alphyn::token<size_t>;
          using type_t = typename token_type::type_t;

          /// \brief possible "types" for a token
          enum e_token_type : type_t
          {
            invalid = ct::alphyn::invalid_token_type,

            // tokens (terminals)
            tok_end         = 0,
            tok_alt         = 1,    // |
            tok_name        = 2,    // rule-name
            tok_terminal    = 3,    // "tadaa" or 'tadaa'
            tok_regexp      = 4,    // regexp
            tok_attribute   = 5,    // [attribute-name]
            tok_br_open     = 6,    // [
            tok_br_close    = 7,    // ]
            tok_line_end    = 8,    // ;
            tok_affect      = 9,    // ::=

            // non-terminals
            start           = 100,
            syntax          = 101,
            rule            = 102,
            expression      = 103,
            list            = 104,
            term            = 105,
          };

          // THE LEXER THINGS //

          // token builders
          template<typename... Entries>
          struct _attr_token_builder
          {
            // search the type in the Type DB
            static constexpr token_type e_id(const char *s, size_t index, size_t end)
            {
              size_t type_index = AttributeDB::size; // invalid
              size_t i = 0;
              NEAM_EXECUTE_PACK(
                (++i) && (type_index == AttributeDB::size) && Entries::strcmp(s, index + 1, end - 1) && (type_index = i - 1)
              );

              if (type_index < AttributeDB::size)
                return token_type {e_token_type::tok_attribute, type_index, s, index, end};
              else
                return token_type::generate_invalid_token(s, index);
            }
          };
          template<typename List>
          using attr_token_builder = typename ct::extract_types<_attr_token_builder, List>::type;

          // regular expressions & strings
          constexpr static string_t s_regexp = "regexp:";
          constexpr static string_t s_affect = "::=";
          constexpr static string_t re_atrribute = "\\[[a-zA-Z0-9:_<>-]+\\]";
          constexpr static string_t re_name = "[a-zA-Z0-9_-]+";
          constexpr static string_t re_terminal_sq = "'[ -&(-~]+'";     // (all ascii except controls and ')
          constexpr static string_t re_terminal_dq = "\"[ !#-~]+\"";    // (all ascii except controls and ")
          constexpr static string_t re_end = "$";

          using lexical_syntax = ct::alphyn::lexical_syntax
          <
            ct::alphyn::syntactic_unit<ct::alphyn::letter<'|'>, token_type, token_type::generate_token_with_type<e_token_type::tok_alt>>,
            ct::alphyn::syntactic_unit<ct::alphyn::letter<';'>, token_type, token_type::generate_token_with_type<e_token_type::tok_line_end>>,
            ct::alphyn::syntactic_unit<ct::alphyn::letter<']'>, token_type, token_type::generate_token_with_type<e_token_type::tok_br_close>>,
            ct::alphyn::syntactic_unit<ct::alphyn::regexp<re_atrribute>, token_type, attr_token_builder<AttributeDB>::e_id>,
            ct::alphyn::syntactic_unit<ct::alphyn::letter<'['>, token_type, token_type::generate_token_with_type<e_token_type::tok_br_open>>,
            ct::alphyn::syntactic_unit<ct::alphyn::string<s_affect>, token_type, token_type::generate_token_with_type<e_token_type::tok_affect>>,
            ct::alphyn::syntactic_unit<ct::alphyn::string<s_regexp>, token_type, token_type::generate_token_with_type<e_token_type::tok_regexp>>,
            ct::alphyn::syntactic_unit<ct::alphyn::regexp<re_name>, token_type, token_type::generate_token_with_type<e_token_type::tok_name>>,
            ct::alphyn::syntactic_unit<ct::alphyn::regexp<re_terminal_sq>, token_type, token_type::generate_token_with_type<e_token_type::tok_terminal>>,
            ct::alphyn::syntactic_unit<ct::alphyn::regexp<re_terminal_dq>, token_type, token_type::generate_token_with_type<e_token_type::tok_terminal>>,
            ct::alphyn::syntactic_unit<ct::alphyn::regexp<re_end>, token_type, token_type::generate_token_with_type<e_token_type::tok_end>>
          >;

          using skipper = ct::alphyn::white_space_skipper;

          using lexer = ct::alphyn::lexer<bnf_meta_parser>;

          // THE PARSER THINGS //

          // synthesizers:
          template<typename...> struct void_synth {using type = int;};

          // handle regexp terminals
          template<typename TokRegexp, typename TokTerminal>
          struct regexp_synth
          {
            struct type
            {
              static constexpr bool is_terminal = true;
              static constexpr bool is_regexp = true;

              static constexpr size_t index = TokTerminal::token.start_index + 1;
              static constexpr size_t end_index = TokTerminal::token.end_index - 1;

              static constexpr size_t stored_string_size = end_index - index;

              // that way I have the storage needed for the string
              template<size_t... Indexes>
              struct string_storage
              {
                static constexpr char str[stored_string_size + 1] = {TokTerminal::token.s[index + Indexes]..., '\0'};
              };
              template<size_t Current, size_t... Indexes> struct string_holder : public string_holder<Current - 1, Current - 1, Indexes...> {};
              template<size_t... Indexes> struct string_holder<0, Indexes...> : public string_storage<Indexes...> {};

              using str_cpy = string_holder<stored_string_size>;
            };
          };

          template<typename TokTerminal>
          struct term_synth
          {
            struct type
            {
              static constexpr bool is_terminal = true;
              static constexpr bool is_regexp = false;

              static constexpr size_t index = TokTerminal::token.start_index + 1;
              static constexpr size_t end_index = TokTerminal::token.end_index - 1;

              static constexpr size_t stored_string_size = end_index - index;

              // that way I have the storage needed for the string
              template<size_t... Indexes>
              struct string_storage
              {
                static constexpr char str[stored_string_size + 1] = {TokTerminal::token.s[index + Indexes]..., '\0'};
              };
              template<size_t Current, size_t... Indexes> struct string_holder : public string_holder<Current - 1, Current - 1, Indexes...> {};
              template<size_t... Indexes> struct string_holder<0, Indexes...> : public string_storage<Indexes...> {};

              using str_cpy = string_holder<stored_string_size>;
            };
          };

          template<typename TokName>
          struct nonterm_synth
          {
            struct type
            {
              static constexpr bool is_terminal = false;
              static constexpr bool is_regexp = false;

              static constexpr size_t index = TokName::token.start_index;
              static constexpr size_t end_index = TokName::token.end_index;

              static constexpr size_t stored_string_size = end_index - index;

              // that way I have the storage needed for the string
              template<size_t... Indexes>
              struct string_storage
              {
                static constexpr char str[stored_string_size + 1] = {TokName::token.s[index + Indexes]..., '\0'};
              };
              template<size_t Current, size_t... Indexes> struct string_holder : public string_holder<Current - 1, Current - 1, Indexes...> {};
              template<size_t... Indexes> struct string_holder<0, Indexes...> : public string_storage<Indexes...> {};

              using str_cpy = string_holder<stored_string_size>;
            };
          };

          template<typename Term> struct create_term_list { using type = ct::type_list<Term>; };
          template<typename Term, typename List> struct append_term_to_list {};
          template<typename Term, typename... ListEntries>
          struct append_term_to_list<Term, ct::type_list<ListEntries...>> { using type = ct::type_list<Term, ListEntries...>; };

          template<typename List, typename Attribute>
          struct create_expression
          {
            struct _entry
            {
              using attribute = typename AttributeDB::template get_type<Attribute::token.value>::type;
              using list = List;
            };
            using type = ct::type_list<_entry>;
          };

          template<typename List, typename Attribute, typename TokAlt, typename Expr>
          struct append_list_to_expression
          {
            struct _entry
            {
              using attribute = typename AttributeDB::template get_type<Attribute::token.value>::type;
              using list = List;
            };
            using type = typename Expr::template prepend<_entry>;
          };

          template<typename TokName, typename TokAffect, typename Expr, typename TokLineEnd>
          struct create_rule
          {
            struct type
            {
              static constexpr size_t index = TokName::token.start_index;
              static constexpr size_t end_index = TokName::token.end_index;

              using expr = Expr;

              static constexpr size_t stored_string_size = end_index - index;

              // that way I have the storage needed for the string
              template<size_t... Indexes>
              struct string_storage
              {
                static constexpr char str[stored_string_size + 1] = {TokName::token.s[index + Indexes]..., '\0'};
              };
              template<size_t Current, size_t... Indexes> struct string_holder : public string_holder<Current - 1, Current - 1, Indexes...> {};
              template<size_t... Indexes> struct string_holder<0, Indexes...> : public string_storage<Indexes...> {};

              using str_cpy = string_holder<stored_string_size>;
            };
          };

          template<typename Rule>
          struct create_syntax
          {
            using type = ct::type_list<Rule>;
          };

          template<typename Rule, typename Syntax>
          struct add_rule_to_syntax
          {
            using type = typename Syntax::template prepend<Rule>;
          };

          // Syntax is a type_list of rules, rules are a type_list of {name / expr},
          // expr is a list of { list / attributes } and list a list of non-/terminals
          //
          // THIS IS THE FINAL BOSS OF THE BNF PARSER.
          //
          template<typename Syntax, typename>
          struct create_output_type
          {
            using token_type = ::neam::ct::alphyn::bnf::token_type;
            using type_t = typename token_type::type_t;

            static constexpr bool strmatch(const char *s, size_t s_index, size_t s_end_index, const char *o, size_t o_index, size_t o_end_index)
            {
              size_t i = s_index;
              size_t j = o_index;
              for (; i < s_end_index && j < o_end_index && s[i] && o[j] && s[i] == o[j]; ++i, ++j);
              if (j < o_end_index || i < s_end_index)
                return false;
              return true;
            }

            // LEXER STUFF
            template<typename X> using _get_expr = typename X::expr;
            template<typename X> using _get_list = typename X::list;
            using global_list = typename Syntax::template direct_for_each<_get_expr>::flatten::template direct_for_each<_get_list>::flatten::make_unique;

            template<typename X> struct is_terminal { static constexpr bool value = X::is_terminal; };

            template<typename X> struct is_regexp { static constexpr bool value = X::is_regexp; };
            template<typename X> struct is_string { static constexpr bool value = !X::is_regexp && ((X::stored_string_size) > 1); };
            template<typename X> struct is_letter { static constexpr bool value = !X::is_regexp && ((X::stored_string_size) == 1); };

            using pre_terminals = typename global_list::template filter_by<is_terminal>;

            template<typename List, const char *String, size_t Index, size_t EndIndex>
            struct get_index_by_str
            {
              template<typename X>
              struct terminal_iterator
              {
                static constexpr bool value = strmatch(String, Index, EndIndex, X::str_cpy::str, 0, X::stored_string_size);
              };
              static constexpr long index = List::template find_if<terminal_iterator>::index;
            };

            template<typename List, typename X>
            struct terminal_filter
            {
              static constexpr bool value = get_index_by_str<List, X::str_cpy::str, 0, X::stored_string_size>::index == -1;
            };

            using terminals = typename pre_terminals::template make_unique_pred<terminal_filter>;

            // split the terminals by the way to match them in a string (string, regexp, letter ?)
            using regexps_terms = typename terminals::template filter_by<is_regexp>;
            using strings_terms = typename terminals::template filter_by<is_string>;
            using letters_terms = typename terminals::template filter_by<is_letter>;

            template<typename X>
            using make_regexp_su = ct::alphyn::syntactic_unit
            <
              ct::alphyn::regexp<X::str_cpy::str>,
              token_type,
              token_type::generate_token_with_type<terminals::template get_type_index<X>::index>
            >;
            template<typename X>
            using make_string_su = ct::alphyn::syntactic_unit
            <
              ct::alphyn::string<X::str_cpy::str>,
              token_type,
              token_type::generate_token_with_type<terminals::template get_type_index<X>::index>
            >;
            template<typename X>
            using make_letter_su = ct::alphyn::syntactic_unit
            <
              ct::alphyn::letter<X::str_cpy::str[0]>,
              token_type,
              token_type::generate_token_with_type<terminals::template get_type_index<X>::index>
            >;

            // construct the syntactic_units from the *_terms
            using regexps_su = typename regexps_terms::template direct_for_each<make_regexp_su>;
            using strings_su = typename strings_terms::template direct_for_each<make_string_su>;
            using letters_su = typename letters_terms::template direct_for_each<make_letter_su>;
            using syntactic_unit_list = typename letters_su::template append_list<strings_su>::template append_list<regexps_su>::make_unique;

            static constexpr size_t base_non_terminal_index = global_list::size + 1000;

            // return the ID of a terminal or a non-terminal
            template<typename X>
            struct get_term_id
            {
              static constexpr long pre_id = (X::is_terminal ? get_index_by_str<terminals, X::str_cpy::str, 0, X::stored_string_size>::index
                                                            : get_index_by_str<Syntax, X::str_cpy::str, 0, X::stored_string_size>::index);

              static_assert(pre_id >= 0, "You may have a non-existent non-terminal in you BNF grammar");

              // the result
              static constexpr size_t id = (X::is_terminal ? 0 : base_non_terminal_index) + pre_id;
            };

            // PARSER STUFF
            template<typename SyntaxClass>
            struct generate_grammar
            {
              // create the production_rules
              template<typename ParsedPR>
              struct pr_maker
              {
                template<typename... TokensOrRules> // TokensOrRules are embed<type_t, ...>'s
                using production_rule = ct::alphyn::production_rule<SyntaxClass, typename ParsedPR::attribute, TokensOrRules::value...>;

                template<typename X>
                using type_t_list = embed::embed<type_t, get_term_id<X>::id>;

                using type = typename ct::extract_types<production_rule, typename ParsedPR::list::template direct_for_each<type_t_list>>::type;
              };

              // create the production_rule_sets
              template<typename ParsedPRS>
              struct prs_maker
              {
                template<typename... Rules>
                using production_rule_set = ct::alphyn::production_rule_set<SyntaxClass, (base_non_terminal_index + Syntax::template get_type_index<ParsedPRS>::index), Rules...>;

                using type = typename ct::extract_types<production_rule_set, typename ParsedPRS::expr::template for_each<pr_maker>>::type;
              };

              template<typename... PRS>
              using pre_grammar = ct::alphyn::grammar<SyntaxClass, base_non_terminal_index, PRS...>;

              using grammar = typename ct::extract_types<pre_grammar, typename Syntax::template for_each<prs_maker>>::type;
            };

            // the output grammar
            struct type
            {
              using token_type = ::neam::ct::alphyn::bnf::token_type;
              using type_t = typename token_type::type_t;

              static const char *get_name_for_token_type(type_t) { return "<[pouet:todo]>"; /* TODO */ }

              // the lexer //

              using lexical_syntax = typename ct::extract_types<ct::alphyn::lexical_syntax, syntactic_unit_list>::type;

              using skipper = ct::alphyn::white_space_skipper; // TODO, make it scriptable too
              using lexer = ct::alphyn::lexer<type>;

              // the parser //

              // shortcuts
              template<typename Attribute, type_t... TokensOrRules>
              using production_rule = ct::alphyn::production_rule<type, Attribute, TokensOrRules...>;
              template<type_t Name, typename... Rules>
              using production_rule_set = ct::alphyn::production_rule_set<type, Name, Rules...>;

              // grammar
              using grammar = typename generate_grammar<type>::grammar;

              // parser
              using parser = ct::alphyn::parser<type>;
            };
          };

          /// \brief Shortcuts for production_rule
          template<typename Attribute, type_t... TokensOrRules>
          using production_rule = ct::alphyn::production_rule<bnf_meta_parser, Attribute, TokensOrRules...>;
          /// \brief Shortcut for production_rule_set
          template<type_t Name, typename... Rules>
          using production_rule_set = ct::alphyn::production_rule_set<bnf_meta_parser, Name, Rules...>;

          /// \brief The parser grammar
          using grammar = ct::alphyn::grammar<bnf_meta_parser, start,
            production_rule_set<start,
              production_rule<ct::alphyn::synthesizer_attribute<create_output_type>, syntax, tok_end>             // syntax -> syntax $
            >,
            production_rule_set<syntax,
              production_rule<ct::alphyn::synthesizer_attribute<create_syntax>, rule>,                            // syntax -> rule
              production_rule<ct::alphyn::synthesizer_attribute<add_rule_to_syntax>, rule, syntax>                // syntax -> syntax rule
            >,
            production_rule_set<rule,
              production_rule<ct::alphyn::synthesizer_attribute<create_rule>, tok_name, tok_affect, expression, tok_line_end>         // rule -> name ::= expression ;
            >,
            production_rule_set<expression,
              production_rule<ct::alphyn::synthesizer_attribute<create_expression>, list, tok_attribute>,                             // expression -> list [attribute-name]
              production_rule<ct::alphyn::synthesizer_attribute<append_list_to_expression>, list, tok_attribute, tok_alt, expression> // expression -> list [attribute-name] | expression
            >,
            production_rule_set<list,
              production_rule<ct::alphyn::synthesizer_attribute<create_term_list>, term>,                         // list -> term
              production_rule<ct::alphyn::synthesizer_attribute<append_term_to_list>, term, list>                 // list -> term list
            >,
            production_rule_set<term,
              production_rule<ct::alphyn::synthesizer_attribute<nonterm_synth>, tok_name>,                        // term -> bla
              production_rule<ct::alphyn::synthesizer_attribute<term_synth>, tok_terminal>,                       // term -> "bla"
              production_rule<ct::alphyn::synthesizer_attribute<regexp_synth>, tok_regexp, tok_terminal>          // term -> regexp:"bla"
            >
          >;

          /// \brief The parser. It parses things.
          using parser = ct::alphyn::parser<bnf_meta_parser>;

          /// \brief A shortcut for generating the parser from a string
          template<const char *BNFString, size_t InitialIndex = 0>
          using generate_parser = typename parser::template ct_parse_string<BNFString, InitialIndex>;
        };

        /// \brief Fast way to use the bnf meta-parser
        /// The \p BNFClass must have \e attributes (type attribute_db) and \e bnf_syntax (static constexpr const char []) defined
        template<typename BNFClass>
        using generate_parser = typename bnf_meta_parser<typename BNFClass::attributes>::template generate_parser<BNFClass::bnf_syntax>;

      } // namespace bnf
    } // namespace alphyn
  } // namespace ct
} //  namespace neam


#endif /*__N_643712118211766787_1135227579__BNF_HPP__*/