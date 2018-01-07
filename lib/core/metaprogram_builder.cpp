
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

#include <metashell/metaprogram_builder.hpp>

#include <metashell/exception.hpp>

#include <boost/algorithm/string/join.hpp>

namespace metashell
{

  metaprogram_builder::metaprogram_builder(
      data::metaprogram::mode_t mode,
      const data::cpp_code& root_name,
      const data::file_location& root_source_location)
    : mp(mode,
         root_name,
         root_source_location,
         data::type_or_code_or_error(
             "Internal Metashell error: metaprogram not finished yet"))
  {
  }

  void metaprogram_builder::handle_macro_expansion_begin(
      const data::cpp_code& name,
      const boost::optional<std::vector<data::cpp_code>>& args,
      const data::file_location& point_of_event,
      const data::file_location& source_location,
      double timestamp)
  {
    data::cpp_code call = name;
    if (args)
    {
      call += "(" + boost::algorithm::join(*args, ",") + ")";
    }

    vertex_descriptor vertex = add_vertex(unique_value(call), source_location);
    vertex_descriptor top_vertex = edge_stack.empty() ?
                                       mp.get_root_vertex() :
                                       mp.get_target(edge_stack.top());

    auto edge =
        mp.add_edge(top_vertex, vertex, data::event_kind::macro_expansion,
                    point_of_event, timestamp);
    edge_stack.push(edge);
  }

  void metaprogram_builder::handle_rescanning(const data::cpp_code& code,
                                              double timestamp)
  {
    if (edge_stack.empty())
    {
      throw exception("Mismatched macro expansion begin and rescanning events");
    }
    auto& ep = mp.get_edge_property(edge_stack.top());

    vertex_descriptor vertex =
        add_vertex(unique_value(code), ep.point_of_event);
    vertex_descriptor top_vertex = mp.get_target(edge_stack.top());

    auto edge = mp.add_edge(top_vertex, vertex, data::event_kind::rescanning,
                            ep.point_of_event, timestamp);
    edge_stack.push(edge);
  }

  void metaprogram_builder::handle_expanded_code(
      const data::cpp_code& code,
      const data::file_location& point_of_event,
      double timestamp)
  {
    vertex_descriptor vertex = add_vertex(unique_value(code), point_of_event);
    vertex_descriptor top_vertex = edge_stack.empty() ?
                                       mp.get_root_vertex() :
                                       mp.get_target(edge_stack.top());

    mp.add_edge(top_vertex, vertex, data::event_kind::expanded_code,
                point_of_event, timestamp);
  }

  void metaprogram_builder::handle_macro_expansion_end(double timestamp)
  {
    // one rescanning and one macro expansion
    for (int i = 0; i != 2; ++i)
    {
      if (edge_stack.empty())
      {
        throw exception("Mismatched macro expansion begin and end events");
      }
      auto& ep = mp.get_edge_property(edge_stack.top());
      ep.time_taken = timestamp - ep.begin_timestamp;

      edge_stack.pop();
    }
  }

  void metaprogram_builder::handle_include_begin(
      const data::include_argument& arg,
      const data::file_location& point_of_event,
      double timestamp)
  {
    vertex_descriptor vertex =
        add_vertex(unique_value(arg.path), data::file_location(arg.path, 1, 1));
    vertex_descriptor top_vertex = edge_stack.empty() ?
                                       mp.get_root_vertex() :
                                       mp.get_target(edge_stack.top());

    auto edge = mp.add_edge(top_vertex, vertex,
                            arg.type == data::include_type::sys ?
                                data::event_kind::sys_include :
                                data::event_kind::quote_include,
                            point_of_event, timestamp);
    edge_stack.push(edge);
  }

  void metaprogram_builder::handle_include_end(double timestamp)
  {
    if (edge_stack.empty())
    {
      throw exception("Mismatched IncludeBegin and IncludeEnd events");
    }
    auto& ep = mp.get_edge_property(edge_stack.top());
    ep.time_taken = timestamp - ep.begin_timestamp;

    edge_stack.pop();
  }

  void metaprogram_builder::handle_define(
      const data::cpp_code& name,
      const boost::optional<std::vector<data::cpp_code>>& args,
      const data::cpp_code& body,
      const data::file_location& point_of_event,
      double timestamp)
  {
    vertex_descriptor vertex = add_vertex(
        unique_value(name +
                     (args ? "(" + boost::algorithm::join(*args, ", ") + ")" :
                             data::cpp_code()) +
                     " " + body),
        point_of_event);
    vertex_descriptor top_vertex = edge_stack.empty() ?
                                       mp.get_root_vertex() :
                                       mp.get_target(edge_stack.top());

    mp.add_edge(top_vertex, vertex, data::event_kind::macro_definition,
                point_of_event, timestamp);
  }

  void metaprogram_builder::handle_undefine(
      const data::cpp_code& name,
      const data::file_location& point_of_event,
      double timestamp)
  {
    vertex_descriptor vertex = add_vertex(unique_value(name), point_of_event);
    vertex_descriptor top_vertex = edge_stack.empty() ?
                                       mp.get_root_vertex() :
                                       mp.get_target(edge_stack.top());

    mp.add_edge(top_vertex, vertex, data::event_kind::macro_deletion,
                point_of_event, timestamp);
  }

  void metaprogram_builder::handle_preprocessing_condition_begin(
      const data::cpp_code& expression,
      const data::file_location& point_of_event,
      double timestamp)
  {
    vertex_descriptor vertex =
        add_vertex(unique_value(expression), point_of_event);
    vertex_descriptor top_vertex = edge_stack.empty() ?
                                       mp.get_root_vertex() :
                                       mp.get_target(edge_stack.top());

    auto edge = mp.add_edge(top_vertex, vertex,
                            data::event_kind::preprocessing_condition,
                            point_of_event, timestamp);
    edge_stack.push(edge);
  }

  void metaprogram_builder::handle_preprocessing_condition_end(bool result,
                                                               double timestamp)
  {
    if (edge_stack.empty())
    {
      throw exception(
          "Mismatched PreprocessingConditionBegin and "
          "PreprocessingConditionEnd events");
    }
    auto& ep = mp.get_edge_property(edge_stack.top());
    ep.time_taken = timestamp - ep.begin_timestamp;

    vertex_descriptor vertex =
        add_vertex(unique_value(data::cpp_code(result ? "true" : "false")),
                   ep.point_of_event);
    vertex_descriptor top_vertex = edge_stack.empty() ?
                                       mp.get_root_vertex() :
                                       mp.get_target(edge_stack.top());

    mp.add_edge(top_vertex, vertex,
                data::event_kind::preprocessing_condition_result,
                ep.point_of_event, timestamp);

    edge_stack.pop();
  }

  void metaprogram_builder::handle_preprocessing_else(
      const data::file_location& point_of_event, double timestamp)
  {
    vertex_descriptor vertex =
        add_vertex(unique_value(data::cpp_code("#else")), point_of_event);
    vertex_descriptor top_vertex = edge_stack.empty() ?
                                       mp.get_root_vertex() :
                                       mp.get_target(edge_stack.top());

    mp.add_edge(top_vertex, vertex, data::event_kind::preprocessing_else,
                point_of_event, timestamp);
  }

  void metaprogram_builder::handle_preprocessing_endif(
      const data::file_location& point_of_event, double timestamp)
  {
    vertex_descriptor vertex =
        add_vertex(unique_value(data::cpp_code("#endif")), point_of_event);
    vertex_descriptor top_vertex = edge_stack.empty() ?
                                       mp.get_root_vertex() :
                                       mp.get_target(edge_stack.top());

    mp.add_edge(top_vertex, vertex, data::event_kind::preprocessing_endif,
                point_of_event, timestamp);
  }

  void metaprogram_builder::handle_error_directive(
      const std::string& message,
      const data::file_location& point_of_event,
      double timestamp)
  {
    vertex_descriptor vertex = add_vertex(
        unique_value(data::cpp_code("#error " + message)), point_of_event);
    vertex_descriptor top_vertex = edge_stack.empty() ?
                                       mp.get_root_vertex() :
                                       mp.get_target(edge_stack.top());

    mp.add_edge(top_vertex, vertex, data::event_kind::error_directive,
                point_of_event, timestamp);
  }

  void metaprogram_builder::handle_line_directive(
      const data::cpp_code& arg,
      const data::file_location& point_of_event,
      const data::file_location& source_location,
      double timestamp)
  {
    vertex_descriptor vertex =
        add_vertex(unique_value("#line " + arg), source_location);
    vertex_descriptor top_vertex = edge_stack.empty() ?
                                       mp.get_root_vertex() :
                                       mp.get_target(edge_stack.top());

    mp.add_edge(top_vertex, vertex, data::event_kind::line_directive,
                point_of_event, timestamp);
  }

  void metaprogram_builder::handle_template_begin(
      data::event_kind kind,
      const data::type& type,
      const data::file_location& point_of_event,
      const data::file_location& source_location,
      double timestamp)
  {
    vertex_descriptor vertex = add_vertex(type, source_location);
    vertex_descriptor top_vertex = edge_stack.empty() ?
                                       mp.get_root_vertex() :
                                       mp.get_target(edge_stack.top());

    auto edge =
        mp.add_edge(top_vertex, vertex, kind, point_of_event, timestamp);
    edge_stack.push(edge);
  }

  void metaprogram_builder::handle_template_end(double timestamp)
  {
    if (edge_stack.empty())
    {
      throw exception(
          "Mismatched Templight TemplateBegin and TemplateEnd events");
    }
    auto& ep = mp.get_edge_property(edge_stack.top());
    ep.time_taken = timestamp - ep.begin_timestamp;

    edge_stack.pop();
  }

  const data::metaprogram& metaprogram_builder::get_metaprogram() const
  {
    if (!edge_stack.empty())
    {
      throw exception("Some Templight TemplateEnd events are missing");
    }
    return mp;
  }

  metaprogram_builder::vertex_descriptor
  metaprogram_builder::add_vertex(const data::metaprogram_node& node,
                                  const data::file_location& source_location)
  {
    element_vertex_map_t::iterator pos;
    bool inserted;

    std::tie(pos, inserted) = element_vertex_map.insert(std::make_pair(
        std::make_tuple(node, source_location), vertex_descriptor()));

    if (inserted)
    {
      pos->second = mp.add_vertex(node, source_location);
    }
    return pos->second;
  }

  void metaprogram_builder::handle_evaluation_end(
      data::type_or_code_or_error result_)
  {
    mp.set_evaluation_result(result_);
  }

  void metaprogram_builder::handle_token_skipping(
      const data::token& token,
      const data::file_location& point_of_event,
      double timestamp)
  {
    vertex_descriptor vertex = add_vertex(unique_value(token), point_of_event);
    vertex_descriptor top_vertex = edge_stack.empty() ?
                                       mp.get_root_vertex() :
                                       mp.get_target(edge_stack.top());

    mp.add_edge(top_vertex, vertex, data::event_kind::skipped_token,
                point_of_event, timestamp);
  }

  void metaprogram_builder::handle_token_generation(
      const data::token& token,
      const data::file_location& point_of_event,
      const data::file_location& source_location,
      double timestamp)
  {
    vertex_descriptor vertex = add_vertex(unique_value(token), source_location);
    vertex_descriptor top_vertex = edge_stack.empty() ?
                                       mp.get_root_vertex() :
                                       mp.get_target(edge_stack.top());

    mp.add_edge(top_vertex, vertex, data::event_kind::generated_token,
                point_of_event, timestamp);
  }
}
