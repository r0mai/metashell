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

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include <metashell/exception.hpp>
#include <metashell/mdb/command.hpp>

namespace metashell {
namespace mdb {

const std::string command::positional_parameter_name =
  "positional_parameter";

void command::add_flag_option(
    char short_name,
    const std::string& name,
    const std::string& docs)
{
  option_t<bool> o = {short_name, name, docs, false};
  flag_options.push_back(o);
}

void command::add_int_option(
    char short_name,
    const std::string& name,
    const std::string& docs,
    int default_value)
{
  option_t<int> o = {short_name, name, docs, default_value};
  int_options.push_back(o);
}

void command::set_positional_option_type(positional_option_t p) {
  positional_option = p;
}

namespace spirit = boost::spirit;
namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;

using skipper_t = qi::space_type;

template<class Iterator>
struct command::command_grammar : qi::grammar<Iterator, skipper_t> {
  command_grammar(const command& self, parsed_command& result) :
    command_grammar::base_type(start),
    self(self),
    result(result)
  {
  }

  const command& self;
  parsed_command& result;

  qi::rule<Iterator, skipper_t> start;
};

parsed_command command::parse_options(const std::string& input) const {
  parsed_command result;

  for (const auto& flag_option : flag_options) {
    assert(result.flag_options.count(flag_option.name) == 0);

    result.flag_options[flag_option.name] = flag_option.default_value;
  }

  for (const auto& int_option : int_options) {
    assert(result.int_options.count(int_option.name) == 0);

    result.int_options[int_option.name] = int_option.default_value;
  }

  command_grammar<std::string::const_iterator> grammar(*this, result);

  auto begin = input.begin();
  const auto end = input.end();

  bool success = qi::phrase_parse(begin, end, grammar, qi::space);

  if (success && begin == end) {
    throw exception("command parsing failed"); // TODO better error message
  }

  return result;
}

}
}
