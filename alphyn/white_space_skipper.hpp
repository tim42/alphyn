//
// file : white_space_skipper.hpp
// in : file:///home/tim/projects/alphyn/alphyn/white_space_skipper.hpp
//
// created by : Timothée Feuillet on linux-vnd3.site
// date: Mon Feb 22 2016 17:55:52 GMT+0100 (CET)
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