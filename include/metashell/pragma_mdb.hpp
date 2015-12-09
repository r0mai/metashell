#ifndef METASHELL_PRAGMA_MDB_HPP
#define METASHELL_PRAGMA_MDB_HPP

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

#include <metashell/shell.hpp>
#include <metashell/command_processor_queue.hpp>
#include <metashell/logger.hpp>
#include <metashell/iface/pragma_handler.hpp>
#include <metashell/iface/executable.hpp>

#include <string>

namespace metashell
{
  class pragma_mdb : public iface::pragma_handler
  {
  public:
    pragma_mdb(
      iface::executable& clang_binary_,
      shell& shell_,
      command_processor_queue* cpq_,
      logger* logger_
    );

    virtual iface::pragma_handler* clone() const override;

    virtual std::string arguments() const override;
    virtual std::string description() const override;

    virtual void run(
      const data::command::iterator& args_begin_,
      const data::command::iterator& args_end_,
      iface::displayer& displayer_
    ) const override;
  private:
    iface::executable& _clang_binary;
    shell& _shell;
    command_processor_queue* _cpq;
    logger* _logger;
  };
}

#endif

