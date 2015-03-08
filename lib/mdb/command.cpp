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

#include <cctype>
#include <cassert>

#include <metashell/exception.hpp>
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

void command::set_positional_option_type(positional_option_t p) {
  positional_option = p;
}

parsed_command command::parse_options(const std::string& input) const {
  parsed_command result;

  fill_with_defaults(result);
  parse_options(input, result)

  return result;
}

void command::fill_with_defaults(parsed_command& result) const {
  for (const auto& flag_option : flag_options) {
    assert(result.flag_options.count(flag_option.name) == 0);

    result.flag_options[flag_option.name] = flag_option.default_value;
  }

  for (const auto& int_option : int_options) {
    assert(result.int_options.count(int_option.name) == 0);

    result.int_options[int_option.name] = int_option.default_value;
  }
}

void command::parse_options(
    const std::string& input, parsed_command& result) const
{
  enum class state_t {
    LAST_TOKEN_WS
  };

  state_t state = state_t::LAST_TOKEN_WS;
  const std::size_t input_size = input.size();

  std::size_t i = 0;

  while (true) {
    if (i >= input_size) {
      // TODO check if we're done, process leftovers
      break;
    }
    parse_next(input, i, result);
  }
}

void command::parse_next_option(
    const std::string& input, std::size_t& i, parsed_command& result) const
{
  const std::size_t input_size = input.size();

  skip_whitespace(input, i);

  if (i >= input_size) {
    return;
  }
  if (input[i] == '-') {
    ++i;
    if (i >= input_size) {
      throw exception("no character after '-'");
    }
    if (input[i] == '-') {
      ++i;
      size_t param_length = 0;
      while (i < input_size && !std::isspace(input[i])) {
        ++param_length;
        ++i;
      }
      if (param_length == 0) {
        throw exception("0 char long -- param");
      }
      // TODO search for param_length param
    } else if (std::isdigit(input[i])) {
      if (positional_option != positional_option_t::NUMERIC) {
        throw exception("negative number without NUMERIC positional");
      }
      // TODO parse number
    } else if (std::isalpha(input[i])) {
      // TODO parse simple -f style params
    }
  }

}

void command::skip_whitespace(const std::string& input, std::size_t& i) const {
  const std::size_t input_size = input.size();
  while (i < input_size && std::isspace(input[i])) {
    ++i;
  }
}

boost::optional<std::string> command::parse_option_name(
    const std::string& input, std::size_t& i) const
{
  if (input[i] == '-') {
    ++i;
    if (i >= input_size) {
      throw exception("no character after '-'");
    }
    if (input[i] == '-') {
      ++i;
      size_t param_length = 0;
      while (i < input_size && !std::isspace(input[i])) {
        ++param_length;
        ++i;
      }
      if (param_length == 0) {
        throw exception("0 char long -- param");
      }
      // TODO search for param_length param
    } else if (std::isdigit(input[i])) {
      if (positional_option != positional_option_t::NUMERIC) {
        throw exception("negative number without NUMERIC positional");
      }
      // TODO parse number
    } else if (std::isalpha(input[i])) {
      // TODO parse simple -f style params
    }
  }
}

}
}
