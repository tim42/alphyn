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
        fallthrough,            ///< \brief The attribute only takes one parameter and return it.
        fallthrough_one,        ///< \brief The attribute takes some parameters and return only one of them.
        value_fallthrough,      ///< \brief The attribute only takes one parameter and return its value. (the parameter must be a token_type/terminal)
        value_fallthrough_one,  ///< \brief The attribute takes some parameters and return the value one only one of them. (that parameter must be a token_type/terminal)
        synthesizer,            ///< \brief The attribute takes some parameters and calls the 'synthesize(...)' static method on a class
      };

      /// \brief Specify an attribute (via function) for the production_rule
      template<typename FunctionType, FunctionType Function, typename...>
      struct attribute {};

      /// \brief Shorthand for the attribute class
      /// \see attribute
#     define ALPHYN_ATTRIBUTE(func)  neam::ct::alphyn::attribute<decltype(func), func>

      // aliases
      template<typename Type>
      using fallthrough_attribute = attribute<e_special_attributes, e_special_attributes::fallthrough, Type>;

      template<size_t Index>
      using fallthrough_one_attribute = attribute<e_special_attributes, e_special_attributes::fallthrough_one, embed::embed<size_t, Index>>;

      template<typename Type>
      using value_fallthrough_attribute = attribute<e_special_attributes, e_special_attributes::value_fallthrough, Type>;

      template<size_t Index>
      using value_fallthrough_one_attribute = attribute<e_special_attributes, e_special_attributes::value_fallthrough_one, embed::embed<size_t, Index>>;

      template<typename Synthesizer>
      using synthesize_attribute = attribute<e_special_attributes, e_special_attributes::synthesizer, Synthesizer>;


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

      // specialisation of attribute for methods
      // NOTE: not tested, may not work.
      template<typename Ret, typename Class, typename... Args, Ret (Class::*Function)(Args...)>
      struct attribute<Ret (Class::*)(Args...), Function>
      {
        using return_type = Ret;
        static constexpr long arity = sizeof...(Args) + 1;
        static constexpr Ret (Class::*_function)(Args...) = Function;

        static constexpr Ret function(Class &c, Args... args)
        {
          return (c.*_function)(std::forward<Args>(args)...);
        }
      };
      // specialisation of attribute for const methods
      // NOTE: not tested, may not work.
      template<typename Ret, typename Class, typename... Args, Ret (Class::*Function)(Args...) const>
      struct attribute<Ret (Class::*)(Args...) const, Function>
      {
        using return_type = Ret;
        static constexpr long arity = sizeof...(Args) + 1;
        static constexpr Ret (Class::*_function)(Args...) const = Function;

        static constexpr Ret function(const Class &c, Args... args)
        {
          return (c.*_function)(std::forward<Args>(args)...);
        }
      };

      // // special attribute handling // //

      // we just forward the attribute
      template<typename Type>
      struct attribute<e_special_attributes, e_special_attributes::fallthrough, Type>
      {
        using return_type = Type;
        static constexpr long arity = 1;

        static constexpr Type function(Type val)
        {
          return std::forward<Type>(val);
        }
      };

      // we just forward the attribute
      template<typename Type>
      struct attribute<e_special_attributes, e_special_attributes::value_fallthrough, Type>
      {
        using return_type = Type;
        static constexpr long arity = 1;

        static constexpr decltype(Type::value) function(Type val)
        {
          return (val.value);
        }
      };

      // we just forward to the Synthesizer class
      template<typename Synthesizer>
      struct attribute<e_special_attributes, e_special_attributes::synthesizer, Synthesizer>
      {
        using return_type = void; // not accounted :/
        static constexpr long arity = 0; // not accounted

        template<typename... Types>
        static constexpr auto function(Types... vals)
        {
          return Synthesizer::synthesize(std::forward<Types>(vals)...);
        }
      };

      // we just forward one attribute
      template<size_t Index>
      struct attribute<e_special_attributes, e_special_attributes::fallthrough_one, embed::embed<size_t, Index>>
      {
        using return_type = void; // not accounted
        static constexpr long arity = -long(Index + 1); // minimum / not accounted

        template<typename RetType, size_t CIndex, typename Current, typename... Other>
        static constexpr RetType _rec(Current c, Other... o)
        {
          if (CIndex == 0)
            return std::forward<RetType>(c);
          return _rec<RetType, CIndex - 1, Other...>(std::forward<Other>(o)...);
        }
        template<typename RetType, size_t CIndex>
        static constexpr RetType _rec()
        {
          static_assert(!(sizeof(RetType) + 1), "fallthrough_one_attribute: Index out of range");
          return RetType();
        }

        template<typename... Types>
        static constexpr auto function(Types... val) -> typename ct::type_at_index<Index, Types...>::type
        {
          using ret_type = typename ct::type_at_index<Index, Types...>::type;
          return _rec<ret_type, Index>();
        }
      };

      // we just forward one attribute's value
      template<size_t Index>
      struct attribute<e_special_attributes, e_special_attributes::value_fallthrough_one, embed::embed<size_t, Index>>
      {
        using return_type = void; // not accounted
        static constexpr long arity = -long(Index + 1); // minimum / not accounted

        template<typename RetType, size_t CIndex, typename Current, typename... Other>
        static constexpr RetType _rec(Current c, Other... o)
        {
          if (CIndex == 0)
            return std::forward<RetType>(c);
          return _rec<RetType, CIndex - 1, Other...>(std::forward<Other>(o)...);
        }
        template<typename RetType, size_t CIndex>
        static constexpr RetType _rec()
        {
          static_assert(!(sizeof(RetType) + 1), "fallthrough_one_attribute: Index out of range");
          return RetType();
        }

        template<typename... Types>
        static constexpr auto function(Types... val) -> decltype(ct::type_at_index<Index, Types...>::type::value)
        {
          using ret_type = typename ct::type_at_index<Index, Types...>::type;
          return _rec<ret_type, Index>().value;
        }
      };
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_17919225691272920340_1595232666__GRAMMAR_ATTRIBUTES_HPP__*/