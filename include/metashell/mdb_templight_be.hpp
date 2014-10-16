#ifndef METASHELL_MDB_TEMPLIGHT_BE_HPP
#define METASHELL_MDB_TEMPLIGHT_BE_HPP

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

#include <tuple>

#include <boost/regex.hpp>
#include <boost/optional.hpp>

#include <metashell/config.hpp>
#include <metashell/metaprogram.hpp>
#include <metashell/mdb_be_base.hpp>
#include <metashell/colored_string.hpp>
#include <metashell/templight_environment.hpp>

namespace metashell {

class mdb_templight_be : public mdb_be_base {
public:
  mdb_templight_be(
      const config& conf,
      const environment& env_arg);

  virtual ~mdb_templight_be() = default;

  virtual void do_continue(int count) override;
  virtual void do_step(step_type type, int count) override;
  virtual void do_evaluate(const std::string& type) override;
  virtual void do_forwardtrace(
      forwardtrace_type type, boost::optional<unsigned> max_depth) override;
  virtual void do_backtrace() override;
  virtual void do_rbreak(const std::string& regex) override;

private:
  // breakpoint is simply a regex for now
  typedef std::tuple<std::string, boost::regex> breakpoint_t;
  typedef std::vector<breakpoint_t> breakpoints_t;

  bool require_evaluated_metaprogram() const;
  bool require_running_metaprogram() const;

  bool breakpoint_match(
      metaprogram::vertex_descriptor vertex, const breakpoint_t& breakpoint);

  void filter_metaprogram();

  bool run_metaprogram_with_templight(const std::string& str);
  boost::optional<std::string> run_metaprogram(const std::string& str);

  breakpoints_t::iterator continue_metaprogram(
      metaprogram::direction_t direction);

  void display_frame(const metaprogram::edge_descriptor& frame) const;
  void display_current_frame() const;
  void display_backtrace() const;
  void display_metaprogram_reached_the_beginning() const;
  void display_metaprogram_finished() const;

  void display_current_forwardtrace(
      boost::optional<unsigned> max_depth) const;
  void display_current_full_forwardtrace(
      boost::optional<unsigned> max_depth) const;

  void display_trace_graph(
      unsigned depth,
      const std::vector<unsigned>& depth_counter,
      bool print_mark) const;

  void display_trace_line(
      metaprogram::vertex_descriptor vertex,
      unsigned depth,
      const std::vector<unsigned>& depth_counter,
      const boost::optional<metaprogram::edge_property>& property,
      unsigned width) const;

  void display_trace_visit(
      metaprogram::optional_edge_descriptor root_edge,
      boost::optional<unsigned> max_depth,
      metaprogram::discovered_t& discovered,
      unsigned width) const;

  config conf;
  templight_environment env;

  boost::optional<metaprogram> mp;
  breakpoints_t breakpoints;

  const static std::string internal_file_name;
  const static std::vector<color> colors;
};

}

#endif
