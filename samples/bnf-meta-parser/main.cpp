
#include <tools/ct_string.hpp>
#include <tools/demangle.hpp>
#include <alphyn.hpp>
#include <default_token.hpp>

#include <iostream>
#include <tuple>
// #include <iomanip>

#include "bnf.hpp"


// the bnf grammar of the tuple builder
constexpr neam::string_t tuple_builder_bnf = R"(
  start ::= '<' expr '>' regexp:'$'     [forward:1];

  expr  ::= val                         [type_to_tuple]
          | expr ',' val                [append_type_to_tuple];

  val   ::= regexp:'[a-zA-Z0-9_-]+'     [token_to_type]
          | '<' expr '>'                [forward:1];
)";

// synthesizers
template<typename TypeDB, template<typename...> class TupleType = std::tuple>
struct tuple_builder
{
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
  // attribute names
  static constexpr neam::string_t token_to_type_str = "token_to_type";
  static constexpr neam::string_t type_to_tuple_str = "type_to_tuple";
  static constexpr neam::string_t append_type_to_tuple_str = "append_type_to_tuple";
  static constexpr neam::string_t forward1_str = "forward:1";

  // the meta attribute-db
  using db = attribute_db<
    attribute_db_entry<token_to_type_str, neam::ct::alphyn::synthesizer_attribute<token_to_type>>,
    attribute_db_entry<type_to_tuple_str, neam::ct::alphyn::synthesizer_attribute<type_to_tuple>>,
    attribute_db_entry<append_type_to_tuple_str, neam::ct::alphyn::synthesizer_attribute<append_type_to_tuple>>,
    attribute_db_entry<forward1_str, neam::ct::alphyn::forward_attribute<1>>
  >;
};

// TEST for the grammar: type DB entry

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

using tuple_builder_parser = bnf_meta_parser<tuple_builder<simple_type_db>::db>::generate_parser<tuple_builder_bnf>;

constexpr neam::string_t tuple_str = "<int, float, uint, int, <long, <long>, double>>";

using result = tuple_builder_parser::parser::ct_parse_string<tuple_str>;

int main(int /*argc*/, char **/*argv*/)
{
  // uncomment the line below to see what nice c++ types are.
  // the second one can be really really huge (some megabytes)
  // std::cout << "generated parser: " << neam::demangle<tuple_builder_parser>() << std::endl;
  // std::cout << "generated automaton: " << neam::demangle<neam::ct::alphyn::grammar_tools<tuple_builder_parser>::lr1_automaton>() << std::endl;

  std::cout << "tuple: " << neam::demangle<result>() << std::endl;

  return 0;
}
