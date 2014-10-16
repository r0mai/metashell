#ifndef METASHELL_MDB_TEST_MDB_FIXTURE_HPP
#define METASHELL_MDB_TEST_MDB_FIXTURE_HPP

// Metashell - Interactive C++ template metaprogramming shell
// Copyright (C) 2014, Andras Kucsma (andras.kucsma@gmail.com)
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

#include <metashell/mdb_templight_be.hpp>

#include "mdb_test_shell.hpp"

struct mdb_fixture {

  mdb_fixture();

private:
  metashell::mdb_templight_be mdb_be;
public:
  mdb_test_shell sh;
};

#endif
