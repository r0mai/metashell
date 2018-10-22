#ifndef METASHELL_MOCK_SHELL_HPP
#define METASHELL_MOCK_SHELL_HPP

// Metashell - Interactive C++ template metaprogramming shell
// Copyright (C) 2018, Abel Sinkovics (abel@sinkovics.hu)
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

#include <metashell/iface/shell.hpp>

#include <gmock/gmock.h>

namespace metashell
{
  namespace mock
  {
    class shell : public iface::shell
    {
    public:
      MOCK_METHOD3(line_available,
                   void(const std::string&,
                        iface::displayer&,
                        iface::history&));

      MOCK_METHOD0(cancel_operation, void());

      MOCK_CONST_METHOD0(prompt, std::string());
      MOCK_CONST_METHOD0(stopped, bool());

      MOCK_METHOD2(code_complete,
                   void(const std::string&, std::set<std::string>&));

      MOCK_CONST_METHOD0(get_config, const data::config&());
      MOCK_METHOD0(get_config, data::config&());

      MOCK_METHOD0(engine, iface::engine&());

      MOCK_METHOD2(store_in_buffer,
                   bool(const data::cpp_code&, iface::displayer&));

      MOCK_CONST_METHOD0(env, const iface::environment&());
      MOCK_METHOD0(env, iface::environment&());

      MOCK_METHOD0(reset_environment, void());
      MOCK_METHOD0(push_environment, void());
      MOCK_METHOD0(pop_environment, void());
      MOCK_METHOD1(display_environment_stack_size, void(iface::displayer&));
      MOCK_METHOD0(rebuild_environment, void());

      MOCK_METHOD3(preprocess,
                   bool(iface::displayer&, const data::cpp_code&, bool));

      MOCK_METHOD2(run_metaprogram,
                   void(const data::cpp_code&, iface::displayer&));

      MOCK_CONST_METHOD0(
          pragma_handlers,
          const std::map<std::vector<std::string>,
                         std::unique_ptr<iface::pragma_handler>>&());

      MOCK_METHOD1(using_precompiled_headers, void(bool));
      MOCK_CONST_METHOD0(using_precompiled_headers, bool());

      MOCK_CONST_METHOD0(env_path, boost::filesystem::path());

      MOCK_METHOD0(stop, void());
    };
  }
}

#endif
