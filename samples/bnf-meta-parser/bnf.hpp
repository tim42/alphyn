//
// file : bnf.hpp
// in : file:///home/tim/projects/alphyn/samples/bnf-meta-parser/bnf.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Mon Mar 14 2016 15:24:35 GMT+0100 (CET)
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

#ifndef __N_643712118211766787_1135227579__BNF_HPP__
# define __N_643712118211766787_1135227579__BNF_HPP__

#include <string> // for the generated get_name_for_token_type()

#include <alphyn.hpp>
#include <default_token.hpp>

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
using attribute_db = neam::ct::type_list<Entries...>;

/// \brief A BNF parser that generates lexer and parser for the input BNF grammar
/// \param TypeDB should be a type_db< db_type_entry< string, type >... >
/// \code bnf_meta_parser<AttributeDB>::generate_parser<BNFString> \endcode will generate the parser for the BNF grammar in BNFString
template<typename AttributeDB>
struct bnf_meta_parser
{
  // token relative things (type and invalid)
  using token_type = neam::ct::alphyn::token<size_t>;
  using type_t = typename token_type::type_t;

  /// \brief possible "types" for a token
  enum e_token_type : type_t
  {
    invalid = neam::ct::alphyn::invalid_token_type,

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
  using attr_token_builder = typename neam::ct::extract_types<_attr_token_builder, List>::type;

  // regular expressions & strings
  constexpr static neam::string_t s_regexp = "regexp:";
  constexpr static neam::string_t s_affect = "::=";
  constexpr static neam::string_t re_atrribute = "\\[[a-zA-Z0-9:_<>-]+\\]";
  constexpr static neam::string_t re_name = "[a-zA-Z0-9_-]+";
  constexpr static neam::string_t re_terminal_sq = "'[ -&(-~]+'";     // (all ascii except controls and ')
  constexpr static neam::string_t re_terminal_dq = "\"[ !#-~]+\"";    // (all ascii except controls and ")
  constexpr static neam::string_t re_end = "$";

  /// \brief The lexical syntax that will be used to tokenize a string (tokens are easier to parse)
  using lexical_syntax = neam::ct::alphyn::lexical_syntax
  <
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<'|'>, token_type, token_type::generate_token_with_type<e_token_type::tok_alt>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<';'>, token_type, token_type::generate_token_with_type<e_token_type::tok_line_end>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<']'>, token_type, token_type::generate_token_with_type<e_token_type::tok_br_close>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::regexp<re_atrribute>, token_type, attr_token_builder<AttributeDB>::e_id>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<'['>, token_type, token_type::generate_token_with_type<e_token_type::tok_br_open>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::string<s_affect>, token_type, token_type::generate_token_with_type<e_token_type::tok_affect>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::string<s_regexp>, token_type, token_type::generate_token_with_type<e_token_type::tok_regexp>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::regexp<re_name>, token_type, token_type::generate_token_with_type<e_token_type::tok_name>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::regexp<re_terminal_sq>, token_type, token_type::generate_token_with_type<e_token_type::tok_terminal>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::regexp<re_terminal_dq>, token_type, token_type::generate_token_with_type<e_token_type::tok_terminal>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::regexp<re_end>, token_type, token_type::generate_token_with_type<e_token_type::tok_end>>
  >;

  /// \brief We simply want to skip white spaces
  using skipper = neam::ct::alphyn::white_space_skipper;

  /// \brief Finally, the lexer
  using lexer = neam::ct::alphyn::lexer<bnf_meta_parser>;

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

  template<typename Term> struct create_term_list { using type = neam::ct::type_list<Term>; };
  template<typename Term, typename List> struct append_term_to_list {};
  template<typename Term, typename... ListEntries>
  struct append_term_to_list<Term, neam::ct::type_list<ListEntries...>> { using type = neam::ct::type_list<Term, ListEntries...>; };

  template<typename List, typename Attribute>
  struct create_expression
  {
    struct _entry
    {
      using attribute = typename AttributeDB::template get_type<Attribute::token.value>::type;
      using list = List;
    };
    using type = neam::ct::type_list<_entry>;
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
    using type = neam::ct::type_list<Rule>;
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
    using token_type = neam::ct::alphyn::token<int>; // the value is unused.
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
    using make_regexp_su = neam::ct::alphyn::syntactic_unit
    <
      neam::ct::alphyn::regexp<X::str_cpy::str>,
      token_type,
      token_type::generate_token_with_type<terminals::template get_type_index<X>::index>
    >;
    template<typename X>
    using make_string_su = neam::ct::alphyn::syntactic_unit
    <
      neam::ct::alphyn::string<X::str_cpy::str>,
      token_type,
      token_type::generate_token_with_type<terminals::template get_type_index<X>::index>
    >;
    template<typename X>
    using make_letter_su = neam::ct::alphyn::syntactic_unit
    <
      neam::ct::alphyn::letter<X::str_cpy::str[0]>,
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
        using production_rule = neam::ct::alphyn::production_rule<SyntaxClass, typename ParsedPR::attribute, TokensOrRules::value...>;

        template<typename X>
        using type_t_list = neam::embed::embed<type_t, get_term_id<X>::id>;

        using type = typename neam::ct::extract_types<production_rule, typename ParsedPR::list::template direct_for_each<type_t_list>>::type;
      };

      // create the production_rule_sets
      template<typename ParsedPRS>
      struct prs_maker
      {
        template<typename... Rules>
        using production_rule_set = neam::ct::alphyn::production_rule_set<SyntaxClass, (base_non_terminal_index + Syntax::template get_type_index<ParsedPRS>::index), Rules...>;

        using type = typename neam::ct::extract_types<production_rule_set, typename ParsedPRS::expr::template for_each<pr_maker>>::type;
      };

      template<typename... PRS>
      using pre_grammar = neam::ct::alphyn::grammar<SyntaxClass, base_non_terminal_index, PRS...>;

      using grammar = typename neam::ct::extract_types<pre_grammar, typename Syntax::template for_each<prs_maker>>::type;
    };

    // the output grammar
    struct type
    {
      using token_type = neam::ct::alphyn::token<int>; // the value is unused.
      using type_t = typename token_type::type_t;

      static std::string get_name_for_token_type(type_t t) { return "<[pouet:todo]>"; /* TODO */ }

      // the lexer //

      using lexical_syntax = typename neam::ct::extract_types<neam::ct::alphyn::lexical_syntax, syntactic_unit_list>::type;

      using skipper = neam::ct::alphyn::white_space_skipper; // TODO, make it scriptable too
      using lexer = neam::ct::alphyn::lexer<type>;

      // the parser //

      // shortcuts
      template<typename Attribute, type_t... TokensOrRules>
      using production_rule = neam::ct::alphyn::production_rule<type, Attribute, TokensOrRules...>;
      template<type_t Name, typename... Rules>
      using production_rule_set = neam::ct::alphyn::production_rule_set<type, Name, Rules...>;

      // grammar
      using grammar = typename generate_grammar<type>::grammar;

      // parser
      using parser = neam::ct::alphyn::parser<type>;
    };
  };

  /// \brief Shortcuts for production_rule
  template<typename Attribute, type_t... TokensOrRules>
  using production_rule = neam::ct::alphyn::production_rule<bnf_meta_parser, Attribute, TokensOrRules...>;
  /// \brief Shortcut for production_rule_set
  template<type_t Name, typename... Rules>
  using production_rule_set = neam::ct::alphyn::production_rule_set<bnf_meta_parser, Name, Rules...>;

  /// \brief The parser grammar
  using grammar = neam::ct::alphyn::grammar<bnf_meta_parser, start,
    production_rule_set<start,
      production_rule<neam::ct::alphyn::synthesizer_attribute<create_output_type>, syntax, tok_end>             // syntax -> syntax $
    >,
    production_rule_set<syntax,
      production_rule<neam::ct::alphyn::synthesizer_attribute<create_syntax>, rule>,                            // syntax -> rule
      production_rule<neam::ct::alphyn::synthesizer_attribute<add_rule_to_syntax>, rule, syntax>                // syntax -> syntax rule
    >,
    production_rule_set<rule,
      production_rule<neam::ct::alphyn::synthesizer_attribute<create_rule>, tok_name, tok_affect, expression, tok_line_end>         // rule -> name ::= expression ;
    >,
    production_rule_set<expression,
      production_rule<neam::ct::alphyn::synthesizer_attribute<create_expression>, list, tok_attribute>,                             // expression -> list [attribute-name]
      production_rule<neam::ct::alphyn::synthesizer_attribute<append_list_to_expression>, list, tok_attribute, tok_alt, expression> // expression -> list [attribute-name] | expression
    >,
    production_rule_set<list,
      production_rule<neam::ct::alphyn::synthesizer_attribute<create_term_list>, term>,                         // list -> term
      production_rule<neam::ct::alphyn::synthesizer_attribute<append_term_to_list>, term, list>                 // list -> term list
    >,
    production_rule_set<term,
      production_rule<neam::ct::alphyn::synthesizer_attribute<nonterm_synth>, tok_name>,                        // term -> bla
      production_rule<neam::ct::alphyn::synthesizer_attribute<term_synth>, tok_terminal>,                       // term -> "bla"
      production_rule<neam::ct::alphyn::synthesizer_attribute<regexp_synth>, tok_regexp, tok_terminal>          // term -> regexp:"bla"
    >
  >;

  /// \brief The parser. It parses things.
  using parser = neam::ct::alphyn::parser<bnf_meta_parser>;

  /// \brief A shortcut for generating the parser from a string
  template<const char *BNFString, size_t InitialIndex = 0>
  using generate_parser = typename parser::template ct_parse_string<BNFString, InitialIndex>;
};


#endif /*__N_643712118211766787_1135227579__BNF_HPP__*/