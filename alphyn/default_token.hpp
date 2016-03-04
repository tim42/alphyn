//
// file : default_token.hpp
// in : file:///home/tim/projects/alphyn/alphyn/default_token.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Mon Feb 22 2016 22:16:36 GMT+0100 (CET)
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

#ifndef __N_267234011226325343_2497132494__DEFAULT_TOKEN_HPP__
# define __N_267234011226325343_2497132494__DEFAULT_TOKEN_HPP__

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      /// \brief The invalid token type for the default token object
      constexpr long invalid_token_type = -1;

      /// \brief A default token class, in the case this is useful for your usage
      /// \note ValueType MUST be default and copy constructible (that's the only requirement on ValueType)
      ///       And if you can make the default and the copy constructors constexpr, that could unleash the power of alphyn ! :)
      template<typename ValueType>
      struct token
      {
        using type_t = long; ///< \brief The type of the type attribute
        using value_t = ValueType; ///< \brief The type of the value attribute

        type_t type = invalid_token_type; ///< \brief A type (usefull for matchin the token against something)
        ValueType value = ValueType(); ///< \brief The value of the token

        const char *s = nullptr;  ///< \brief The string the token is originating from
        size_t start_index = 0; ///< \brief The index in the above string
        size_t end_index = 0; ///< \brief The final index in the above string

        // // static helpers // //

        /// \brief Helper that generates an invalid token
        /// \note a token class must have this function
        static constexpr token generate_invalid_token(const char *s = nullptr, size_t start_position = 0)
        {
          return token {invalid_token_type, ValueType(), s, start_position, size_t(-1)};
        }

        /// \brief An helper when you just have to set the type of a token
        /// you can use this method when in a syntactic_rule you just want to set the type of the token, not a value
        template<long Type>
        static constexpr token generate_token_with_type(const char *s, size_t index, size_t end)
        {
          return token {Type, ValueType(), s, index, end};
        }


        // // mandatories methods // //
        constexpr bool is_valid() const
        {
          return type != invalid_token_type;
        }

        constexpr bool operator == (const token &o) const
        {
          return this->type == o.type;
        }

        constexpr bool operator != (const token &o) const
        {
          return this->type != o.type;
        }

        constexpr bool operator == (type_t t) const
        {
          return this->type == t;
        }

        constexpr bool operator != (type_t t) const
        {
          return this->type != t;
        }
      };
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_267234011226325343_2497132494__DEFAULT_TOKEN_HPP__*/