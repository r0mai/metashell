#ifndef METASHELL_MDB_SHELL_HPP
#define METASHELL_MDB_SHELL_HPP

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

#include <string>

#include <boost/optional.hpp>

#include <metashell/colored_string.hpp>
#include <metashell/mdb_command_handler_map.hpp>

namespace metashell {

class mdb_be_base;

class mdb_shell {
public:
  const static mdb_command_handler_map command_handler;

  mdb_shell(mdb_be_base& mdb_be);

  virtual ~mdb_shell() = default;

  virtual void run() = 0;

  virtual void add_history(const std::string& str) = 0;

  virtual void display(
      const colored_string& cs,
      colored_string::size_type first,
      colored_string::size_type length) const = 0;

  void display(const colored_string& cs) const;

  virtual unsigned width() const = 0;

  std::string prompt() const;
  bool stopped() const;

  void display_splash() const;
  void line_available(const std::string& line);

  void command_continue(const std::string& arg);
  void command_step(const std::string& arg);
  void command_evaluate(const std::string& arg);
  void command_forwardtrace(const std::string& arg);
  void command_backtrace(const std::string& arg);
  void command_rbreak(const std::string& arg);
  void command_help(const std::string& arg);
  void command_quit(const std::string& arg);

  void display_error(const std::string& str) const;
  void display_info(const std::string& str) const;
  void display_argument_parsing_failed() const;
protected:

  bool require_empty_args(const std::string& args) const;

  mdb_be_base& mdb_be;

  std::string prev_line;
  bool last_command_repeatable = false;

  bool is_stopped = false;

};

}

#endif
