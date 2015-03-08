#ifndef METASHELL_MDB_COMMAND_HPP
#define METASHELL_MDB_COMMAND_HPP

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

#include <string>
#include <vector>

#include <metashell/mdb/parsed_command.hpp>

namespace metashell {
namespace mdb {

class command {
public:
  typedef std::vector<std::string> words_t;

  enum class positional_option_t {
    NONE, TYPE, NUMERIC
  };

  void add_flag_option(
      char short_name,
      const std::string& name,
      const std::string& docs);
  void add_int_option(
      char short_name,
      const std::string& name,
      const std::string& docs,
      int default_value);

  void set_positional_option_type(positional_option_t p);

  parsed_command parse_options(const std::string& input) const;

private:
  const static std::string positional_parameter_name;

  template<class T>
  struct option_t {
    char short_name; // unused if short_name == 0
    std::string name;
    std::string docs;
    T default_value;
  };

  typedef std::vector<option_t<bool>> flag_options_t;
  typedef std::vector<option_t<int>> int_options_t;

  void tokenize_input(const std::string& str) const;

  flag_options_t flag_options;
  int_options_t int_options;
  positional_option_t positional_option = positional_option_t::NONE;
};

}
}

#endif
