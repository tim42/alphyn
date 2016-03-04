//
// file : grammar_attributes.hpp
// in : file:///home/tim/projects/alphyn/alphyn/grammar_attributes.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Tue Mar 01 2016 11:29:37 GMT+0100 (CET)
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
        using return_type = void;
        static constexpr long arity = -long(Index + 1); // minimum / not accounted

        template<e_forward_mode = e_forward_mode::token_value>
        struct function_t
        {
          static constexpr size_t index = Index;
        };
        static constexpr function_t<> function = function_t<>();
      };
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_17919225691272920340_1595232666__GRAMMAR_ATTRIBUTES_HPP__*/