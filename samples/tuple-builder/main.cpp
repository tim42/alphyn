
#include <tools/ct_string.hpp>
#include <tools/demangle.hpp>
#include <alphyn.hpp>
#include <default_token.hpp>

#include <iostream>
#include <tuple>
// #include <iomanip>


template<const char *TypeName, typename Type>
struct db_type_entry
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
using type_db = neam::ct::type_list<Entries...>;

/// \brief a very simple tuple builder
/// \param TypeDB should be a type_db< db_type_entry< string, type >... >
template<typename TypeDB, template<typename...> class TupleType = std::tuple>
struct tuple_builder
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
    tok_id          = 1,
    tok_comma       = 2,
    tok_br_open     = 3,
    tok_br_close    = 4,

    // non-terminals
    start   = 100,
    expr    = 101,
    val     = 104,
  };

  // THE LEXER THINGS //

  // token builders
  template<typename... Entries>
  struct _id_token_builder
  {
    // search the type in the Type DB
    static constexpr token_type e_id(const char *s, size_t index, size_t end)
    {
      size_t type_index = TypeDB::size; // invalid
      size_t i = 0;
      NEAM_EXECUTE_PACK(
        (++i) && (type_index == TypeDB::size) && Entries::strcmp(s, index, end) && (type_index = i - 1)
      );

      if (type_index < TypeDB::size)
        return token_type {e_token_type::tok_id, type_index, s, index, end};
      else
        return token_type::generate_invalid_token(s, index);
    }
  };
  template<typename List>
  using id_token_builder = typename neam::ct::extract_types<_id_token_builder, List>::type;

  // regular expressions
  constexpr static neam::string_t re_id = "[a-zA-Z0-9_-]+";
  constexpr static neam::string_t re_end = "$";

  /// \brief The lexical syntax that will be used to tokenize a string (tokens are easier to parse)
  using lexical_syntax = neam::ct::alphyn::lexical_syntax
  <
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<','>, token_type, token_type::generate_token_with_type<e_token_type::tok_comma>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<'<'>, token_type, token_type::generate_token_with_type<e_token_type::tok_br_open >>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<'>'>, token_type, token_type::generate_token_with_type<e_token_type::tok_br_close >>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::regexp<re_id>, token_type, id_token_builder<TypeDB>::e_id>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::regexp<re_end>, token_type, token_type::generate_token_with_type<e_token_type::tok_end>>
  >;

  /// \brief We simply want to skip white spaces
  using skipper = neam::ct::alphyn::white_space_skipper;

  /// \brief Finally, the lexer
  using lexer = neam::ct::alphyn::lexer<tuple_builder>;

  // THE PARSER THINGS //

  // synthesizers:
  template<typename TokID>
  struct token_to_type          // it transforms a token to a type
  {
    using type = typename TypeDB::template get_type<TokID::token.value>::type;
  };

  template<typename Type>
  struct type_to_tuple          // it transforms a single type to a tuple
  {
    using type = TupleType<Type>;
  };

  template<typename Tuple, typename TokComma, typename Type>
  struct append_type_to_tuple {};

  template<typename... TupleEntries, typename TokComma, typename Type>
  struct append_type_to_tuple<TupleType<TupleEntries...>, TokComma, Type> // it appends a type to a tuple
  {
    using type = TupleType<TupleEntries..., Type>;
  };

  /// \brief Shortcuts for production_rule
  template<typename Attribute, type_t... TokensOrRules>
  using production_rule = neam::ct::alphyn::production_rule<tuple_builder, Attribute, TokensOrRules...>;
  /// \brief Shortcut for production_rule_set
  template<type_t Name, typename... Rules>
  using production_rule_set = neam::ct::alphyn::production_rule_set<tuple_builder, Name, Rules...>;

  /// \brief The parser grammar
  using grammar = neam::ct::alphyn::grammar<tuple_builder, start,
    production_rule_set<start,
      production_rule<neam::ct::alphyn::forward_attribute<1>, tok_br_open, expr, tok_br_close, tok_end>      // start -> < expr > $
    >,

    production_rule_set<expr,
      production_rule<neam::ct::alphyn::synthesizer_attribute<type_to_tuple>, val>,                         // expr -> val
      production_rule<neam::ct::alphyn::synthesizer_attribute<append_type_to_tuple>, expr, tok_comma, val>  // expr -> expr, val
    >,

    production_rule_set<val,
      production_rule<neam::ct::alphyn::synthesizer_attribute<token_to_type>, tok_id>,                      // val -> id
      production_rule<neam::ct::alphyn::forward_attribute<1>, tok_br_open, expr, tok_br_close>              // val -> < expr >
    >
  >;

  /// \brief The parser. It parses things.
  using parser = neam::ct::alphyn::parser<tuple_builder>;
};

// create a simple type DB
constexpr neam::string_t int_str = "int";
constexpr neam::string_t uint_str = "uint";
constexpr neam::string_t long_str = "long";
constexpr neam::string_t ulong_str = "ulong";
constexpr neam::string_t float_str = "float";
constexpr neam::string_t double_str = "double";

using simple_type_db = type_db
<
  db_type_entry<int_str, int>,
  db_type_entry<uint_str, unsigned int>,
  db_type_entry<long_str, long>,
  db_type_entry<ulong_str, unsigned long>,
  db_type_entry<float_str, float>,
  db_type_entry<double_str, double>
>;

// a test string:
constexpr neam::string_t tuple_str = "<int, float, uint, int, <long, long, double>>";

using res_tuple = tuple_builder<simple_type_db>::parser::ct_parse_string<tuple_str>;                           // create a std::tuple from the string
using res_type_list = tuple_builder<simple_type_db, neam::ct::type_list>::parser::ct_parse_string<tuple_str>;  // create a neam::ct::type_list from the same string

// check that the tuple is correct:
static_assert(std::is_same<res_tuple, std::tuple<int, float, unsigned int, int, std::tuple<long, long, double>>>::value, "invalid tuple type (did you change the string ?)");

int main(int /*argc*/, char **/*argv*/)
{
  std::cout << "result tuple: " << neam::demangle<res_tuple>() << std::endl;
  std::cout << "result type list: " << neam::demangle<res_type_list>() << std::endl;
  return 0;
}
