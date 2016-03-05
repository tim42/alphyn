

### how to use

To have a lexer or a parser, you have to create a specific type (`class` or `struct`) that defines the following types / has the following static properties:

for a lexer:
```c++
struct my_syntax_class
{
  // TOKEN RELATIVE THINGS: //

  // You have to define a token type. Alphyn comes with a default but in most regards correct
  // implementation of a token class. That token class asks for the type of the value to store
  // (must be default constructible).
  // Unless you have some serious need to do so, the default implementation will be alright.
  // Both the lexer and parser is designed to work with any token type provided it meets some
  // (small) requirements. (see the neam::ct::alphyn::token class for more information)
  using token_type = neam::ct::alphyn::token<long>;

  // This is absolutely not mandatory, but that allow to have some pretty code
  // that can benefit from documentation (you may also have macros, direct integer values, ...)
  enum e_token_type : token_type::type_t
  {
    invalid = neam::ct::alphyn::invalid_token_type,

    // tokens
    tok_end,
    tok_number,
    tok_add
  };

  // LEXER THINGS: //

  // This is a token builder.
  // When you token must have a specific value (here an atoi() like operation is performed),
  // you may define a function that will be called in order to create a token from a part of the
  // string to tokenize. (s is the string to tokenize, index is the start index, end the end index)
  // You may declare the functions elsewhere, but I prefer them here for clarity purposes.
  static constexpr token_type e_number(const char *s, size_t index, size_t end)
  {
    long value = 0;
    for (size_t i = index; i < end; ++i)
      value = value * 10 + (s[i] - '0');
    return token_type {e_token_type::tok_number, value, s, index, end};
  }

  // Regular expressions.
  // Alphyn comes with a pretty standard (and compile-time) regular expression "engine",
  // so you can use some complex regular expression, but remember, the more complex is
  // the regular expression, the more instruction your compiler will generate for the lexer.
  // You may declare the strings elsewhere, but I prefer them here for clarity purposes.
  // The only restriction is that they MUST be constexpr.
  constexpr static neam::string_t re_number = "[0-9]+";
  constexpr static neam::string_t re_end = "$";         // match the end of the input

  // The lexical syntax that will be used to tokenize a string
  // You list as template parameters all possible tokens (please note that they are tested from top to bottom)
  // a neam::ct::alphyn::syntactic_unit<> define a possible token type.
  // Its first parameter is something that will be used to match the token from the input string.
  // It could be letter<'x'> (or letter<'x', 'X', ...> if there are one-letter alternatives),
  // string<my_string_goes_here> it simply do a strcmp-like comparison,
  // regexpr<my_regexp_goes_here> for a more complete (and possibly slow) comparison
  // Next is the token type that will be generated (should always be token_type)
  // and after is the function that will be called to create the token (a constexpr function whose source is accessible is a good choice).
  // If you simply want to set a type for the token and discard the value, you may use the token_type::generate_token_with_type<e_token_type::my_token_type_goes_here>
  using lexical_syntax = neam::ct::alphyn::lexical_syntax
  <
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::letter<'+'>, token_type, token_type::generate_token_with_type<e_token_type::tok_add>>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::regexp<re_number>, token_type, e_number>,
    neam::ct::alphyn::syntactic_unit<neam::ct::alphyn::regexp<re_end>, token_type, token_type::generate_token_with_type<e_token_type::tok_end>>
  >;

  // Here we simply want to skip white spaces.
  // You have to define a skipper type in your class. But if you don't
  // want a skipper you can always set it to neam::ct::alphyn::skip_syntax<>.
  // No code will then be generated for the skipper.
  using skipper = neam::ct::alphyn::white_space_skipper;

  // And finally, the lexer. (absolutely not mandatory, but that's a good shorthand for it).
  // The lexer defines two way to access the tokens:
  // lexer::ct_lexem_list<my_string_goes_here>  <- purely compile-time
  // and
  // lexer::get_lazy_lexer(my_string_goes_here) <- still compile-time, but can also be used at runtime
  // The result are quite the same and for both cases the tokens are generated when needed
  // (either by the compiler or by some runtime code).
  // For more information, neam::ct::alphyn::lexe is documented.
  using lexer = neam::ct::alphyn::lexer<my_syntax_class>;
};
```

for a parser you have to extend the previous class with the following properties / types:
```c++
struct my_syntax_class
{
  // same as the lexer syntax class
  // same as the lexer syntax class
};
```
