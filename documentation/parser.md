
# ALPHYN PARSER

## The parser definition

I will assume that you've read [that almost english thing on the lexer](lexer.md).

The parser job is to transform the list of token that the lexer has output and transform it into something called an AST (abstract syntax tree).
The whole lexer/parse job can be summed-up by *"it takes a string as input and output something a computer can use"*.

To create a parser with alphyn, you must have a class that defines a lexer and extend it with the grammar.
Here we will use `math_eval`, the one defined [here](lexer.md).

```c++
struct math_eval
{
```
*All the lexer stuff goes here*

This is a convenient shorthand for production rules. It is not mandatory, but cleans a bit the following code
```c++
  template<typename Attribute, token_type::type_t... TokensOrRules>
  using production_rule = neam::ct::alphyn::production_rule<math_eval, Attribute, TokensOrRules...>;
```

This is also a convenient shorthand for sets of production rules
```c++
  template<token_type::type_t Name, typename... Rules>
  using production_rule_set = neam::ct::alphyn::production_rule_set<math_eval, Name, Rules...>;
```

We are a mathematical expression evaluator, so here are the different operations:
The `const token_type &` at the second position is simply the token of the operator
```c++
  static constexpr float attr_add(float n1, const token_type &, float n2) { return n1 + n2; }
  static constexpr float attr_sub(float n1, const token_type &, float n2) { return n1 - n2; }
  static constexpr float attr_mul(float n1, const token_type &, float n2) { return n1 * n2; }
  static constexpr float attr_div(float n1, const token_type &, float n2) { return n1 / n2; }
```


And the grammar.

Alphyn uses attributes to extend the grammar. It also define some currently used attributes like
`forward_attribute<Index>` that forwards the value of a given terminal or non-terminal as result of the production.
For the production `val -> ( sum )` it forwards the value of `sum` as the value of `val`.

There's also `value_forward_attribute<Index>` that forwards the value of a token.

But for every other cases, you may use `ALPHYN_ATTRIBUTE(&my_function)`.
Your function arity must matches with the production rule and the return type must be different of void.
You may use any return type you want, but they must be default-constructible and copy-assignable. An attribute may not be a template function.
There's no check and no cast performed on the parameter type, so please be consistent with your return type for a given
non-terminal if you don't want your parser to spuriously fails.
If you do dynamic allocation, please keep in mind that alphyn calls the destructors at the very end of the parsing process,
and existing objects may be re-used with a copy-assignation.

So. The grammar.

`neam::ct::alphyn::grammar<math_eval, start,` tells the grammar what is the class holding the grammar and the start symbol.

The grammar consists of `production_rule_set` holding `production_rule`s.
A `production_rule_set` is the definition of a non-terminal. **All** the production rules
that produces a non-terminal must be in a single `production_rule_set`.
The first parameter of a `production_rule_set` is the non-terminal that will be produced by the set of rules.

`production_rule` represent the left-hand of a production. (In `prod -> prod / val` it is `prod / val`).
It consists of an attribute (function or predefined) and a list of terminals and non-terminals.

```c++
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
```

The parser could then be defined.
`neam::ct::alphyn::on_parse_error::print_message` specify that we want to print a message when a string can't be parsed.
The default is to throw an exception.
```c++
  using parser = neam::ct::alphyn::parser<math_eval, neam::ct::alphyn::on_parse_error::print_message>;
```

```c++
}
```

## Source code

See [here](../samples/test/main.cpp).

## How to use the parser

`math_eval::parser::parse_string`. This is a compile-time and runtime solution for parsing a string and get the result back.

It takes a template parameter that is the return type of the parser (in the case of the math_eval class, it would have been <float> as we work on floating point numbers).

Its parameters are a string and an optional offset in the string.
For the math_eval case, a typical invocation of the parser would be
```c++
float result = math_eval::parser::parse_string<float>("0.5 + 0.5");
```

If both your attributes and functions the generate tokens (a lexer thing) are `constexpr`, you are eligible to ask alphyn to perform at compile-time.
You can then do

```c++
static_assert(float result = math_eval::parser::parse_string<float>("1+1") == 2, "Either the world has became wrong or alphyn has a problem. (please check the world)");
```
