
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
    tok_par_open,
    tok_par_close,

    // non-terminal
    start,
    expr,
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
      case math_eval::tok_par_open: return "tok_par_open";
      case math_eval::tok_par_close: return "tok_par_close";
      case math_eval::start: return "[start]";
      case math_eval::expr: return "[expr]";
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
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<'('>, token_type, token_type::generate_token_with_type<e_token_type::tok_par_open>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<')'>, token_type, token_type::generate_token_with_type<e_token_type::tok_par_close>>,
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

  /// \brief The parser grammar
  using grammar = neam::ct::alphyn::grammar<math_eval, start,
    production_rule_set<start,
      production_rule<neam::ct::alphyn::forward_first_attribute, sum, tok_end>      // start -> sum
    >,

    production_rule_set<sum,
      production_rule<neam::ct::alphyn::forward_first_attribute, prod>,             // sum -> prod
      production_rule<ALPHYN_ATTRIBUTE(&attr_add), sum, tok_add, prod>,             // sum -> sum + prod
      production_rule<ALPHYN_ATTRIBUTE(&attr_sub), sum, tok_sub, prod>              // sum -> sum - prod
    >,
    production_rule_set<prod,
      production_rule<neam::ct::alphyn::forward_first_attribute, val>,              // prod -> val
      production_rule<ALPHYN_ATTRIBUTE(&attr_mul), prod, tok_mul, val>,             // prod -> prod * val
      production_rule<ALPHYN_ATTRIBUTE(&attr_div), prod, tok_div, val>              // prod -> prod / val
    >,

    production_rule_set<val,
      production_rule<neam::ct::alphyn::value_forward_first_attribute, tok_number>,               // val -> number
      production_rule<neam::ct::alphyn::forward_attribute<1>, tok_par_open, sum, tok_par_close>   // val -> ( sum )
    >
  >;

  /// \brief The parser. It parses things.
  using parser = neam::ct::alphyn::parser<math_eval, neam::ct::alphyn::on_parse_error::print_message/*call_error_handler*/>;

  /// \brief A default handler
  template<typename ReturnType>
  static ReturnType on_parse_error(const char *string, size_t index)
  {
    static_assert(std::is_same<ReturnType, float>::value, "math_eval: the return type must be <float>");
    std::cerr << "Could not parse the string: " << (string + index) << std::endl;
    return ReturnType(-1);
  }
};

// create a compile-time lexem list
constexpr neam::string_t test_str = "10 + 5 * 2";
auto initial_token = math_eval::lexer::ct_lexem_list<test_str>::token;


int main(int /*argc*/, char **/*argv*/)
{
//   std::cout << "automaton: \n";
//   neam::ct::alphyn::debug_printer<math_eval>::print_graph();
//   std::cout << "size of the automaton: " << grammar_tool::lr1_automaton::as_type_list::size << " states\n";

  // the proof that alphyn is compile-time:
//   static_assert(math_eval::parser::parse_string<float>("2.5 * 4.0 + 4 / 2 + 4 * 2") == 20, "Well... The parser / grammar / string / ... is not OK");
//   static_assert(math_eval::parser::ct_parse_string<float, test_str>::result == 20, "Well... The parser / grammar / string / ... is not OK");

  std::cout << "res: " << math_eval::parser::parse_string<float>("(1+1) * 2.5 + 5 / 2 * (3 - 0.5)") << '\n';

//   return 0;

  // speed test //
  neam::cr::chrono chr;
  std::string expr = "1";
  // it generates a string with one-character tokens, some separated by white-space tokens
  for (size_t i = 0; i < 130 * 1000 * 1000; ++i) expr += " + 0 * 1";
  double dtime = chr.delta();
  std::cout << "generated an expression of " << (expr.size() / 1000 / 1000) << "MToken [" << dtime << "s]" << std::endl;
  std::cout << "TEST1: " << math_eval::parser::parse_string<float>(expr.c_str()) << '\n';
  double acctime = chr.get_accumulated_time();
  std::cout << "dtime: " << acctime << "s [" << (float(expr.size() / 1000 / 1000) / acctime) << "MToken/s]" << std::endl;
  std::cout << "  -> " << "parser is " << (acctime / dtime) << "x slower than the generation"<< std::endl;
  return 0;
}
