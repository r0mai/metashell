#ifndef METASHELL_DATA_UNKNOWN_STANDARD_HPP
#define METASHELL_DATA_UNKNOWN_STANDARD_HPP

// Metashell - Interactive C++ template metaprogramming shell
// Copyright (C) 2019, Abel Sinkovics (abel@sinkovics.hu)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <metashell/data/exception.hpp>
#include <metashell/data/real_engine_name.hpp>

#include <string>

namespace metashell
{
  namespace data
  {
    class unknown_standard : public exception
    {
    public:
      unknown_standard(const real_engine_name&, const std::string&);
    };
  }
}

#endif
