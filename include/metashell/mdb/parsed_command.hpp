#ifndef METASHELL_MDB_PARSED_COMMAND_HPP
#define METASHELL_MDB_PARSED_COMMAND_HPP

// Metashell - Interactive C++ template metaprogramming shell
// Copyright (C) 2015, Andras Kucsma (andras.kucsma@gmail.com)
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

#include <map>
#include <string>

namespace metashell {
namespace mdb {

class parsed_command {
  friend class command;
public:

  bool get_flag_option(const std::string& name) const;

private:
  typedef std::map<std::string, bool> flag_options_t;
  typedef std::map<std::string, int> int_options_t;

  flag_options_t flag_options;
  int_options_t int_options;
};

}
}

#endif
