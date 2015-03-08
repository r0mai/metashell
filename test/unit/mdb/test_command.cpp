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

#include <metashell/mdb/command.hpp>

#include <just/test.hpp>

using namespace metashell;

JUST_TEST_CASE(test_mdb_command_parsing_int_option) {
  mdb::command cmd;
  cmd.add_int_option('s', "steps", "docs", 0);

  mdb::parsed_command parsed_cmd = cmd.parse_options("--steps 32");
  JUST_ASSERT_EQUAL(32, parsed_cmd.get_int_option("steps"));
}

JUST_TEST_CASE(test_mdb_command_parsing_int_options_default) {
  mdb::command cmd;
  cmd.add_int_option('s', "steps", "docs", 10);

  mdb::parsed_command parsed_cmd = cmd.parse_options("");
  JUST_ASSERT_EQUAL(10, parsed_cmd.get_int_option("steps"));
}

JUST_TEST_CASE(test_mdb_command_parsing_flag_option) {
  mdb::command cmd;
  cmd.add_flag_option('f', "full", "docs");

  mdb::parsed_command parsed_cmd = cmd.parse_options("--full");
  JUST_ASSERT(parsed_cmd.get_flag_option("full"));
}

JUST_TEST_CASE(test_mdb_command_parsing_flag_options_default) {
  mdb::command cmd;
  cmd.add_flag_option('f', "full", "docs");

  mdb::parsed_command parsed_cmd = cmd.parse_options("");
  JUST_ASSERT(!parsed_cmd.get_flag_option("full"));
}
