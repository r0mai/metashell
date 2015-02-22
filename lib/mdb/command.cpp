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

#include <cassert>

#include <metashell/mdb/command.hpp>

namespace metashell {
namespace mdb {

const std::string command::positional_parameter_name =
  "positional_parameter";

void command::add_flag_option(
    const std::string& name,
    const std::string& docs,
    bool default_value)
{
  using boost::program_options::bool_switch;

  assert(flag_options.count(name) == 0);

  flag_options[name] = default_value;

  options.add_options()(
      name.c_str(),
      bool_switch(&flag_options[name])->default_value(default_value),
      docs.c_str());
}

void command::add_numeric_positional_option(int default_value) {
  using boost::program_options::value;

  assert(int_options.count(positional_parameter_name) == 0);

  int_options[positional_parameter_name] = default_value;

  options.add_options()(
      positional_parameter_name.c_str(),
      value<int>(&int_options[positional_parameter_name])->
      default_value(default_value));

  positional_options.add(positional_parameter_name.c_str(), 1);
}

bool command::get_flag_option(const std::string& name) const {
  assert(flag_options.count(name) != 0);

  return flag_options.at(name);
}

int command::get_numeric_positional_option() const {
  assert(int_options.count(positional_parameter_name) != 0);

  return int_options.at(positional_parameter_name);
}

command::words_t command::get_words() {
  return words;
}

}
}
