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

#include <metashell/mdb_shell.hpp>
#include <metashell/mdb_be_base.hpp>

namespace metashell {

void mdb_be_base::set_shell(mdb_shell *new_shell) {
  shell = new_shell;
}

void mdb_be_base::do_continue(int /*count*/) {
  display_unsupported_command();
}

void mdb_be_base::do_step(step_type /*type*/, int /*count*/) {
  display_unsupported_command();
}

void mdb_be_base::do_evaluate(const std::string& /*type*/) {
  display_unsupported_command();
}

void mdb_be_base::do_forwardtrace(
    forwardtrace_type /*type*/, boost::optional<unsigned> /*max_depth*/)
{
  display_unsupported_command();
}

void mdb_be_base::do_backtrace() {
  display_unsupported_command();
}

void mdb_be_base::do_rbreak(const std::string& /*regex*/) {
  display_unsupported_command();
}

void mdb_be_base::display_unsupported_command() {
  shell->display_error("This command is not supported in this backend");
}

}
