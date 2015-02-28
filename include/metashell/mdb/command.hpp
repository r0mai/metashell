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

#include <boost/program_options.hpp>

namespace metashell {
namespace mdb {

class command {
public:
  typedef std::vector<std::string> words_t;

  void add_flag_option(
      const std::string& name,
      const std::string& docs);
  void add_int_option(
      const std::string& name,
      const std::string& docs,
      int default_value);

  // These are mutually exclusive
  void add_numeric_positional_option(int default_value);
  void add_type_positional_option();

  words_t get_words();

  parsed_command parse_options(const std::string& input) const;

private:
  const static std::string positional_parameter_name;

  template<class T>
  struct option_t {
    std::string name;
    std::string docs;
    T default_value;
  };

  struct positional_option_t {
    std::string name;
    int max_count;
  };

  typedef std::vector<option_t<bool>> flag_options_t;
  typedef std::vector<option_t<int>> int_options_t;
  typedef std::vector<positional_option_t> positional_options_t;

  flag_options_t flag_options;
  int_options_t int_options;
  positional_options_t positional_options;

  bool has_type_position_option = false;

  words_t words;
};

}
}

#endif
