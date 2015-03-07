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
    const std::string& docs)
{
  option_t<bool> o = {name, docs, false};
  flag_options.push_back(o);
}

void command::add_int_option(
    const std::string& name,
    const std::string& docs,
    int default_value)
{
  option_t<int> o = {name, docs, default_value};
  int_options.push_back(o);
}

void command::add_numeric_positional_option(int default_value) {
  assert(positional_options.empty());
  assert(!has_type_position_option);

  using boost::program_options::value;

  add_int_option(positional_parameter_name, "no-docs-yet", default_value);

  positional_option_t positional_option = {positional_parameter_name, 1};
  positional_options.push_back(positional_option);
}

void command::add_type_positional_option() {
  assert(positional_options.empty());
  has_type_position_option = true;
}

parsed_command command::parse_options(const std::string& input) const {
  namespace po = boost::program_options;

  po::positional_options_description positional_options_desc;
  po::options_description program_options;

  parsed_command result;

  for (const auto& flag_option : flag_options) {
    assert(result.flag_options.count(flag_option.name) == 0);

    result.flag_options[flag_option.name] = flag_option.default_value;

    program_options.add_options()(
        flag_option.name.c_str(),
        po::bool_switch(&result.flag_options[flag_option.name])->
          default_value(flag_option.default_value),
        flag_option.docs.c_str());
  }

  for (const auto& int_option : int_options) {
    assert(result.int_options.count(int_option.name) == 0);

    result.int_options[int_option.name] = int_option.default_value;

    program_options.add_options()(
        int_option.name.c_str(),
        po::value<int>(&result.int_options[int_option.name])->
          default_value(int_option.default_value),
        int_option.docs.c_str());
  }

  for (const auto& positional_option : positional_options) {
    positional_options_desc.add(
        positional_option.name.c_str(),
        positional_option.max_count
    );
  }

  //TODO actually tokenize and parse

  return result;
}

}
}
