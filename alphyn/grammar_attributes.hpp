//
// file : grammar_attributes.hpp
// in : file:///home/tim/projects/alphyn/alphyn/grammar_attributes.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Tue Mar 01 2016 11:29:37 GMT+0100 (CET)
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

#ifndef __N_17919225691272920340_1595232666__GRAMMAR_ATTRIBUTES_HPP__
# define __N_17919225691272920340_1595232666__GRAMMAR_ATTRIBUTES_HPP__

#include <utility>
#include <tools/embed.hpp>

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      /// \brief Some special attributes
      enum class e_special_attributes
      {
        forward,            ///< \brief The attribute sets the result of the production rule to one of production elements.
        value_forward,      ///< \brief The attribute sets the result of the production rule to one of production element's value.
        synthesizer,        ///< \brief Special case of the ct-only parser. This allows working on types instead of values.
                            ///         Does not work in parser::parse_string("bla");
                            ///         The synthesizer is a template class that define has a `type` type, which is the result of the synthesizer.
      };

      template<template<typename... X> class Synthesizer>
      struct wrap_synthesizer
      {
        template<typename... T>
        using synthesizer = Synthesizer<T...>;
      };

      /// \brief Specify an attribute (via function) for the production_rule
      template<typename FunctionType, FunctionType Function, typename...>
      struct attribute {};

      /// \brief Shorthand for the attribute class
      /// \see attribute
#     define ALPHYN_ATTRIBUTE(func)  neam::ct::alphyn::attribute<decltype(func), func>

      // aliases
      template<size_t Index = 0>
      using forward_attribute = attribute<e_special_attributes, e_special_attributes::forward, embed::embed<size_t, Index>>;
      using forward_first_attribute = forward_attribute<0>;

      template<size_t Index = 0>
      using value_forward_attribute = attribute<e_special_attributes, e_special_attributes::value_forward, embed::embed<size_t, Index>>;
      using value_forward_first_attribute = value_forward_attribute<0>;
      template<template<typename... X> class Synthesizer>
      using synthesizer_attribute = attribute<e_special_attributes, e_special_attributes::synthesizer, wrap_synthesizer<Synthesizer>>;

      // specialisation of attribute for functions
      template<typename Ret, typename... Args, Ret (*Function)(Args...)>
      struct attribute<Ret (*)(Args...), Function>
      {
        using return_type = Ret;

        // void as return-type in attribute functions is not allowed
        static_assert(!std::is_same<Ret, void>::value, "attribute return type is void");

        static constexpr long arity = sizeof...(Args);
        static constexpr Ret function(Args... args)
        {
          return Function(std::forward<Args>(args)...);
        }
      };

      // // special attribute handling // //

      // internal. (TODO: make an internal namespace)
      enum class e_forward_mode
      {
        direct,
        token_value
      };

      // we just forward one attribute
      template<size_t Index>
      struct attribute<e_special_attributes, e_special_attributes::forward, embed::embed<size_t, Index>>
      {
        using return_type = void; // not accounted
        static constexpr long arity = -long(Index + 1); // minimum / not accounted

        template<e_forward_mode = e_forward_mode::direct>
        struct function_t
        {
          static constexpr size_t index = Index;
        };
        static constexpr function_t<> function = function_t<>();
      };

      // we just forward one attribute's value
      template<size_t Index>
      struct attribute<e_special_attributes, e_special_attributes::value_forward, embed::embed<size_t, Index>>
      {
        using return_type = void; // not accounted
        static constexpr long arity = -long(Index + 1); // minimum / not accounted

        template<e_forward_mode = e_forward_mode::token_value>
        struct function_t
        {
          static constexpr size_t index = Index;
        };
        static constexpr function_t<> function = function_t<>();
      };
      // synthesizer special case
      template<typename SynthesizerWrapper>
      struct attribute<e_special_attributes, e_special_attributes::synthesizer, SynthesizerWrapper>
      {
        using return_type = void;           // not accounted
        static constexpr long arity = 0;    // not accounted
      };
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_17919225691272920340_1595232666__GRAMMAR_ATTRIBUTES_HPP__*/