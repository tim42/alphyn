
#include <tools/ct_string.hpp>
#include <tools/chrono.hpp>
#include <alphyn.hpp>
#include <default_token.hpp>

#include <iostream>
// #include <iomanip>

#include "debug.hpp"

/// \brief a simple mathematical evaluator
struct math_eval
{
  // token relative things (type and invalid)
  using token_type = neam::ct::alphyn::token<float>;
  using type_t = token_type::type_t;

  /// \brief possible type for a token
  enum e_token_type : type_t
  {
    invalid = neam::ct::alphyn::invalid_token_type,

    // tokens
    tok_end,
    tok_number,
    tok_add,
    tok_sub,
    tok_mul,
    tok_div,
//     tok_par_open,
//     tok_par_close,

    start,
    sum,
    prod,
    val,
  };

  static std::string get_name_for_token_type(type_t t)
  {
    switch (t)
    {
      case math_eval::tok_end: return "tok_end";
      case math_eval::tok_number: return "tok_number";
      case math_eval::tok_add: return "tok_add";
      case math_eval::tok_sub: return "tok_sub";
      case math_eval::tok_mul: return "tok_mul";
      case math_eval::tok_div: return "tok_div";
//       case math_eval::tok_par_open: return "tok_par_open";
//       case math_eval::tok_par_close: return "tok_par_close";
      case math_eval::start: return "[start]";
      case math_eval::sum: return "[sum]";
      case math_eval::prod: return "[prod]";
      case math_eval::val: return "[val]";
    }
    return "[invalid]";
  }

  // THE LEXER THINGS //

  // token builders
  static constexpr token_type e_number(const char *s, size_t index, size_t end)
  {
    // a small strtof
    float value = 0;
    size_t i = index;
    for (; s[i] != '.' && i < end; ++i)
      value = value * 10 + (s[i] - '0');
    if (s[i] == '.')
    {
      for (size_t j = i + 1; j < end; ++j)
        value += float(s[j] - '0') / float((j - i) * 10);
    }
    return token_type {e_token_type::tok_number, value, s, index, end};
  }

  // regular expressions
  constexpr static neam::string_t re_number = "[0-9]+(\\.[0-9]*)?";
  constexpr static neam::string_t re_end = "$";

  /// \brief The lexical syntax that will be used to tokenize a string (tokens are easier to parse)
  using lexical_syntax = neam::ct::alphyn::lexical_syntax
  <
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<'+'>, token_type, token_type::generate_token_with_type<e_token_type::tok_add>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<'-'>, token_type, token_type::generate_token_with_type<e_token_type::tok_sub>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<'*'>, token_type, token_type::generate_token_with_type<e_token_type::tok_mul>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<'/'>, token_type, token_type::generate_token_with_type<e_token_type::tok_div>>,
//     neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<'('>, token_type, token_type::generate_token_with_type<e_token_type::tok_par_open>>,
//     neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<')'>, token_type, token_type::generate_token_with_type<e_token_type::tok_par_close>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::regexp<re_number>, token_type, e_number>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::regexp<re_end>, token_type, token_type::generate_token_with_type<e_token_type::tok_end>>
  >;

  /// \brief We simply want to skip white spaces
  using skipper = neam::ct::alphyn::white_space_skipper;

  /// \brief Finally, the lexer
  using lexer = neam::ct::alphyn::lexer<math_eval>;

  // THE PARSER THINGS //

  /// \brief Shortcut for production_rule
  template<typename Attribute, token_type::type_t... TokensOrRules>
  using production_rule = neam::ct::alphyn::production_rule<math_eval, Attribute, TokensOrRules...>;
  /// \brief Shortcut for production_rule_set
  template<token_type::type_t Name, typename... Rules>
  using production_rule_set = neam::ct::alphyn::production_rule_set<math_eval, Name, Rules...>;

  /// \brief Because of the value_fallthrough_attribute we work on float directly
  static constexpr float attr_add(float n1, const token_type &, float n2) { return n1 + n2; }
  static constexpr float attr_sub(float n1, const token_type &, float n2) { return n1 - n2; }
  static constexpr float attr_mul(float n1, const token_type &, float n2) { return n1 * n2; }
  static constexpr float attr_div(float n1, const token_type &, float n2) { return n1 / n2; }

  static constexpr float attr_fwd(const token_type &, float n2) { return n2; }
  static constexpr float attr_fwd1(float n2, const token_type &) { return n2; }

  /// \brief The parser grammar
  using grammar = neam::ct::alphyn::grammar<math_eval, start,
    production_rule_set<start, production_rule<ALPHYN_ATTRIBUTE(&attr_fwd1), sum, tok_end>>,

    production_rule_set<sum,
      production_rule<neam::ct::alphyn::fallthrough_attribute<long>, prod>,
      production_rule<ALPHYN_ATTRIBUTE(&attr_add), sum, tok_add, prod>,
      production_rule<ALPHYN_ATTRIBUTE(&attr_sub), sum, tok_sub, prod>
    >,
    production_rule_set<prod,
      production_rule<neam::ct::alphyn::fallthrough_attribute<long>, val>,
      production_rule<ALPHYN_ATTRIBUTE(&attr_mul), prod, tok_mul, val>,
      production_rule<ALPHYN_ATTRIBUTE(&attr_div), prod, tok_div, val>
    >,

    production_rule_set<val,
      production_rule<neam::ct::alphyn::value_fallthrough_attribute<token_type>, tok_number>
    >
  >;

  /// \brief The parser
  using parser = neam::ct::alphyn::parser<math_eval>;
};

// create a compile-time lexem list
constexpr neam::string_t test_str = "10 + 5 * 2";
auto initial_token = math_eval::lexer::ct_lexem_list<test_str>::token;


int main(int /*argc*/, char **/*argv*/)
{
//   std::cout << "automaton: \n";
//   neam::ct::alphyn::debug_printer<math_eval>::print_graph();
//   std::cout << "size of the automaton: " << grammar_tool::lr1_automaton::as_type_list::size << " states\n";

  // Oh YEAH: (the proof that alphyn is compile-time !)
  static_assert(math_eval::parser::parse_string<float>("2.5 * 4.0 + 2 / 1 + 4 * 2") == 20, "Well... The parser / grammar / string / ... is not OK");
  static_assert(math_eval::parser::ct_parse_string<float, test_str>::result == 20, "Well... The parser / grammar / string / ... is not OK");

  std::cout << "res: " << math_eval::parser::parse_string<float>("10 + 2.5 * 2 * 5 / 2") << '\n';

  // speed test //
  std::string expr = "1";
  // it generates a string with one-character tokens, some separated by white-space tokens
  for (size_t i = 0; i < 130 * 1000 * 1000; ++i) expr += " + 0 * 1";
  std::cout << "generated an expression of " << (expr.size() / 1000 / 1000) << "MToken" << std::endl;
  neam::cr::chrono chr;
  std::cout << "TEST1: " << math_eval::parser::parse_string<float>(expr.c_str()) << '\n';
  std::cout << "dtime: " << chr.get_accumulated_time() << "s [" << (float(expr.size() / 1000 / 1000) / chr.get_accumulated_time()) << "MToken/s]" << std::endl;

  return 0;
}
