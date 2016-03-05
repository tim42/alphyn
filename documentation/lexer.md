
# ALPHYN LEXER

## The lexer definition

The lexer's job is to split a string into a list of "token".

For the string `56 + 8*3`, a lexer for mathematical expression will create the following list of tokens: `56`, `+`, `8`, `*` and `3`.

Alphyn needs a lexer in order to have a parser. So, here is how you create a lexer for simple mathematical expressions with alphyn:

```c++
#include <alphyn.hpp>
```

Alpyn needs a single class or struct to have a working lexer and parser:
```c++
struct math_eval
{
```

This tells alphyn what kind of token you want the lexer to create. As we don't have special needs, we simply use the default token class
and tell alphyn that this token will hold floating point as optional value. (The default token class can hold any default-constructible objects)
```c++
  using token_type = neam::ct::alphyn::token<float>;
```

This `enum` is not mandatory, but this nicely describe the different terminals and non-terminals
the syntax and the grammar will have.
```c++
  using type_t = typename token_type::type_t;
  enum e_token_type : type_t
  {
    invalid = neam::ct::alphyn::invalid_token_type,

    // terminals
    tok_end,
    tok_number,
    tok_add, tok_sub, tok_mul, tok_div,
    tok_par_open, tok_par_close,

    // non-terminals
    start, expr,
    sum, prod, val,
  };
```

This isn't mandatory, it's simply to have possibly nice error messages
```c++
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
```

Here we enter in the realm of the lexer.

This function is a small utility that transform the range of a string into a floating-point number.
It is called by the lexer when some special token needs to be generated.
You can write constexpr and non-constexpr functions, but I tend to prefer constexpr ones for simple tasks.
It is not mandatory to have those functions as members (you can use any function with the correct signature).
```c++
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
```

Then we have regular expression. Again, you can put them anywhere you like, the only restriction is
it **must** be `constexpr neam::string_t`. (`neam::string_t` is an alias to `char[]`)

Please also note that even if you may use some complex regular expressions, they are transformed into code
at the compilation, so for complex regexp, more code will be generated (it this will possibly be slower).
The regular expressions used by alphyn are greedy.
```c++
  // a regular expression for matching numbers:
  constexpr static neam::string_t re_number = "[0-9]+(\\.[0-9]*)?";
  // and one for matching the end of the input:
  constexpr static neam::string_t re_end = "$";
```

This part is mandatory: it's what the lexer will use to transform a string into a list of token.
We will break down a line to see what this mean:
 - `neam::ct::alphyn::syntactic_unit<...>`: this type will hold all the information alphyn needs to generate a particular token.
   You may have more than one for a single token type.
 - `neam::ct::alphyn::letter<'+'>` or `neam::ct::alphyn::letter<'(', '['>`
   or `neam::ct::alphyn::regexp<re_number>` or `neam::ct::alphyn::string<str_keyword_while>`:
   this describe what letter, string or regular expression must match in order to generate a token.
   `letter` matches if one of the letter it has as argument is equal to the current character of the input
   `string` matches if there is a substring at the current position of the input that is equal to the one it has as argument
   `regexp` matches if the regexp string matches something at the current position of the input
 - `token_type`: mandatory. It tells the lexical syntax the token_type used by the lexer.
 - `token_type::generate_token_with_type<e_token_type::tok_add>`: If you use the default token class,
   this will create a token with the corresponding type and no value.
 - `e_number`: If you need a value, you must give a function that will create a token for the lexer.
```c++
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
```

The skipper (mandatory) will tell to the lexer what kind of token is to skip.
Here, we just want to skip white spaces. You can disable the skipper by setting it to
`using skypper = neam::ct::alphyn::skip_syntax<>`. You can also define your own syntax
for tokens to ignore by using some `skip_unit<letter<' ', '\t', '\n'>>`.
A `skip_unit` is like a `syntactic_unit` but it only takes a matcher (letter, string or regexp).
```c++
  using skipper = neam::ct::alphyn::white_space_skipper;
```

Here is the definition of the lexer.
```c++
  using lexer = neam::ct::alphyn::lexer<math_eval>;
```

For the parser, see [here](parser.md)

```c++
};
```

## How to use the lexer

If you intend to use a parser, this step is automatically done by alphyn and you don't have to worry about it.

There's two way to use the lexer. You can both use it to construct a "recursive" list of token during the build
or asking it to generate a list of token on the fly (at both compile-time and runtime).

`math_eval::lexer::get_lazy_lexer(const char *string, size_t start_index = 0)` will mostly be the only function you will use.
It returns an instance of `math_eval::lexer::lazy_lexem_list` with the first token.
The lexing process is done only when you call `get_next()` on a lazy_lexem_list.
You can query for the token by using `get_token()` and check if the token is the last token with `is_last()`.

`math_eval::lexer::ct_lexem_list<const char *String, size_t StartIndex = 0>` is also here in the case you absolutely need
an overkill compile-time token list that fails the compilation on syntax error.
