//
// file : white_space_skipper.hpp
// in : file:///home/tim/projects/alphyn/alphyn/white_space_skipper.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Mon Feb 22 2016 17:55:52 GMT+0100 (CET)
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

#ifndef __N_15829324212571418681_1410214124__WHITE_SPACE_SKIPPER_HPP__
# define __N_15829324212571418681_1410214124__WHITE_SPACE_SKIPPER_HPP__

#include "lexer_skip.hpp"

namespace neam
{
  namespace ct
  {
    namespace alphyn
    {
      /// \brief A skipper that skip white spaces (' ', '\t', '\n')
      using white_space_skipper = skip_syntax<skip_unit<letter<' ', '\t', '\n'>>>;
    } // namespace alphyn
  } // namespace ct
} // namespace neam

#endif /*__N_15829324212571418681_1410214124__WHITE_SPACE_SKIPPER_HPP__*/