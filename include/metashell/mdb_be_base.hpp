#ifndef METASHELL_MDB_BE_BASE_HPP
#define METASHELL_MDB_BE_BASE_HPP

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

#include <memory>
#include <string>
#include <type_traits>

#include <boost/optional.hpp>

namespace metashell {

class mdb_shell;

class mdb_be_base {
public:

  mdb_be_base() = default;

  virtual ~mdb_be_base() = default;

  void set_shell(mdb_shell *new_shell);

  enum class step_type { normal, over };
  enum class forwardtrace_type { normal, full };

  virtual void do_continue(int count);
  virtual void do_step(step_type type, int count);
  virtual void do_evaluate(const std::string& type);
  virtual void do_forwardtrace(
      forwardtrace_type type, boost::optional<unsigned> max_depth);
  virtual void do_backtrace();
  virtual void do_rbreak(const std::string& regex);

protected:
  void display_unsupported_command();

  mdb_shell *shell;
};

}

#endif
