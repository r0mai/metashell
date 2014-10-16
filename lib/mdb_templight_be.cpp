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

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <cmath>

#include <metashell/mdb_templight_be.hpp>
#include <metashell/highlight_syntax.hpp>
#include <metashell/metashell.hpp>
#include <metashell/temporary_file.hpp>
#include <metashell/is_template_type.hpp>
#include <metashell/mdb_shell.hpp>

namespace metashell {

const std::string mdb_templight_be::internal_file_name = "mdb-stdin";

const std::vector<color> mdb_templight_be::colors =
  {
    color::red,
    color::green,
    color::yellow,
    color::blue,
    color::cyan
  };

mdb_templight_be::mdb_templight_be(
    const config& conf,
    const environment& env_arg) :
  conf(conf),
  env("__mdb_internal", conf)
{
  env.append(env_arg.get_all());
}

void mdb_templight_be::do_continue(int count) {
  if (!require_evaluated_metaprogram()) {
    return;
  }

  if (count == 0) {
    return;
  }

  metaprogram::direction_t direction =
    count >= 0 ? metaprogram::forward : metaprogram::backwards;

  breakpoints_t::iterator breakpoint_it = breakpoints.end();
  for (int i = 0;
      i < std::abs(count) && !mp->is_at_endpoint(direction); ++i)
  {
    breakpoint_it = continue_metaprogram(direction);
  }

  if (mp->is_finished()) {
    if (count > 0) {
      display_metaprogram_finished();
    }
  } else if (mp->is_at_start()) {
    if (count < 0) {
      display_metaprogram_reached_the_beginning();
    }
  } else {
    assert(breakpoint_it != breakpoints.end());
    shell->display_info(
        "Breakpoint \"" + std::get<0>(*breakpoint_it) + "\" reached\n");
    display_current_frame();
  }
}

void mdb_templight_be::do_step(step_type type, int count) {
  if (!require_evaluated_metaprogram()) {
    return;
  }

  metaprogram::direction_t direction =
    count >= 0 ? metaprogram::forward : metaprogram::backwards;

  int iteration_count = std::abs(count);

  switch (type) {
    case step_type::normal:
      for (int i = 0;
          i < iteration_count && !mp->is_at_endpoint(direction); ++i)
      {
        mp->step(direction);
      }
      break;
    case step_type::over:
      {
        for (int i = 0;
            i < iteration_count && !mp->is_at_endpoint(direction); ++i)
        {
          unsigned bt_depth = mp->get_backtrace_length();
          do {
            mp->step(direction);
          } while (!mp->is_at_endpoint(direction) &&
              mp->get_backtrace_length() > bt_depth);
        }
      }
      break;
    default:
      assert(false);
      break;
  }

  if (mp->is_finished()) {
    if (count > 0) {
      display_metaprogram_finished();
    }
  } else if (mp->is_at_start()) {
    if (count < 0) {
      display_metaprogram_reached_the_beginning();
    }
  } else {
    display_current_frame();
  }
}

void mdb_templight_be::do_evaluate(const std::string& type_ref) {
  std::string type = type_ref;
  if (type.empty()) {
    if (!mp) {
      shell->display_error("Nothing has been evaluated yet.\n");
      return;
    }
    type = mp->get_vertex_property(mp->get_root_vertex()).name;
  }

  breakpoints.clear();

  if (!run_metaprogram_with_templight(type)) {
    return;
  }
  shell->display_info("Metaprogram started\n");

  filter_metaprogram();
}

void mdb_templight_be::do_forwardtrace(
    forwardtrace_type type, boost::optional<unsigned> max_depth) {
  if (!require_running_metaprogram()) {
    return;
  }

  if (type == forwardtrace_type::full) {
    display_current_full_forwardtrace(max_depth);
  } else {
    display_current_forwardtrace(max_depth);
  }
}

void mdb_templight_be::do_backtrace() {
  if (!require_running_metaprogram()) {
    return;
  }
  display_backtrace();
}

void mdb_templight_be::do_rbreak(const std::string& regex) {
  if (regex.empty()) {
    shell->display_error("Argument expected\n");
    return;
  }
  if (!require_running_metaprogram()) {
    return;
  }
  try {
    breakpoint_t breakpoint = std::make_tuple(regex, boost::regex(regex));

    unsigned match_count = 0;
    for (metaprogram::vertex_descriptor vertex : mp->get_vertices()) {
      if (breakpoint_match(vertex, breakpoint)) {
        match_count += mp->get_enabled_in_degree(vertex);
      }
    }
    if (match_count == 0) {
      shell->display_info(
          "Breakpoint \"" + regex + "\" will never stop the execution\n");
    } else {
      shell->display_info(
          "Breakpoint \"" + regex + "\" will stop the execution on " +
          std::to_string(match_count) +
          (match_count > 1 ? " locations" : " location") + "\n");
      breakpoints.push_back(breakpoint);
    }
  } catch (const boost::regex_error&) {
    shell->display_error("\"" + regex + "\" is not a valid regex\n");
  }
}

bool mdb_templight_be::require_evaluated_metaprogram() const {
  if (!mp) {
    shell->display_error("Metaprogram not evaluated yet\n");
    return false;
  }
  return true;
}

bool mdb_templight_be::require_running_metaprogram() const {
  if (!require_evaluated_metaprogram()) {
    return false;
  }

  if (mp->is_finished()) {
    display_metaprogram_finished();
    return false;
  }
  return true;
}

bool mdb_templight_be::breakpoint_match(
    metaprogram::vertex_descriptor vertex, const breakpoint_t& breakpoint)
{
  return boost::regex_search(
      mp->get_vertex_property(vertex).name, std::get<1>(breakpoint));
}

void mdb_templight_be::filter_metaprogram() {
  assert(mp);

  using boost::starts_with;
  using boost::ends_with;
  using boost::trim_copy;

  using vertex_descriptor = metaprogram::vertex_descriptor;
  using edge_descriptor = metaprogram::edge_descriptor;
  using edge_property = metaprogram::edge_property;
  using discovered_t = metaprogram::discovered_t;

  static const std::string wrap_prefix = "metashell::impl::wrap<";
  static const std::string wrap_suffix = ">";

  // TODO this check could be made more strict,
  // since we now whats inside wrap<...> (mp->get_evaluation_result)
  auto is_wrap_type = [](const std::string& type) {
    return starts_with(type, wrap_prefix) && ends_with(type, wrap_suffix);
  };

  std::string env_buffer = env.get();
  int line_number = std::count(env_buffer.begin(), env_buffer.end(), '\n');

  // First disable everything
  for (edge_descriptor edge : mp->get_edges()) {
    mp->get_edge_property(edge).enabled = false;
  }

  // We will traverse the interesting edges later
  std::stack<edge_descriptor> edge_stack;

  // Enable the interesting root edges
  for (edge_descriptor edge : mp->get_out_edges(mp->get_root_vertex())) {
    edge_property& property = mp->get_edge_property(edge);
    const std::string& target_name =
      mp->get_vertex_property(mp->get_target(edge)).name;
    // Filter out edges, that is not instantiated by the entered type
    if (property.point_of_instantiation.name == internal_file_name &&
        property.point_of_instantiation.row == line_number + 2 &&
        (property.kind == instantiation_kind::template_instantiation ||
        property.kind == instantiation_kind::memoization) &&
        (!is_wrap_type(target_name) ||
         property.kind != instantiation_kind::memoization))
    {
      property.enabled = true;
      edge_stack.push(edge);
    }
  }

  discovered_t discovered(mp->get_num_vertices());
  // Traverse the graph to enable all edges which are reachable from the
  // edges enabled above
  while (!edge_stack.empty()) {
    edge_descriptor edge = edge_stack.top();
    edge_stack.pop();

    assert(mp->get_edge_property(edge).enabled);

    vertex_descriptor vertex = mp->get_target(edge);

    if (discovered[vertex]) {
      continue;
    }

    for (edge_descriptor out_edge : mp->get_out_edges(vertex)) {
      edge_property& property = mp->get_edge_property(out_edge);
      if (property.kind == instantiation_kind::template_instantiation ||
         property.kind == instantiation_kind::memoization)
      {
        property.enabled = true;
        edge_stack.push(out_edge);
      }
    }
  }

  // Unwrap vertex names
  for (metaprogram::vertex_descriptor vertex : mp->get_vertices()) {
    std::string& name = mp->get_vertex_property(vertex).name;
    if (is_wrap_type(name)) {
      name = trim_copy(name.substr(
          wrap_prefix.size(),
          name.size() - wrap_prefix.size() - wrap_suffix.size()));
      if (!is_template_type(name)) {
        for (metaprogram::edge_descriptor in_edge : mp->get_in_edges(vertex)) {
          mp->get_edge_property(in_edge).kind =
            instantiation_kind::non_template_type;
        }
      }
    }
  }

  // Clang sometimes produces equivalent instantiations events from the same
  // point. Filter out all but one of each
  for (metaprogram::vertex_descriptor vertex : mp->get_vertices()) {

    typedef std::tuple<file_location, instantiation_kind, vertex_descriptor>
      set_element_t;

    std::set<set_element_t> similar_edges;

    for (metaprogram::edge_descriptor edge : mp->get_out_edges(vertex)) {
      metaprogram::edge_property& edge_property =
        mp->get_edge_property(edge);

      set_element_t set_element = std::make_tuple(
            edge_property.point_of_instantiation,
            edge_property.kind,
            mp->get_target(edge));

      auto it = similar_edges.find(set_element);
      if (it != similar_edges.end()) {
        edge_property.enabled = false;
      } else {
        similar_edges.insert(set_element);
      }
    }
  }
}

bool mdb_templight_be::run_metaprogram_with_templight(
    const std::string& str)
{
  temporary_file templight_xml_file("templight-%%%%-%%%%-%%%%-%%%%.xml");
  std::string xml_path = templight_xml_file.get_path().string();

  env.set_xml_location(xml_path);

  boost::optional<std::string> evaluation_result = run_metaprogram(str);

  if (!evaluation_result) {
    mp = boost::none;
    return false;
  }

  mp = metaprogram::create_from_xml_file(xml_path, str, *evaluation_result);
  return true;
}

boost::optional<std::string> mdb_templight_be::run_metaprogram(
    const std::string& str)
{
  result res = eval_tmp_unformatted(env, str, conf, internal_file_name);

  if (!res.info.empty()) {
    shell->display_info(res.info);
  }

  if (res.has_errors()) {
    for (const std::string& e : res.errors) {
      shell->display_error(e + "\n");
    }
    return boost::none;
  }
  return res.output;
}

mdb_templight_be::breakpoints_t::iterator
mdb_templight_be::continue_metaprogram(metaprogram::direction_t direction) {
  assert(!mp->is_at_endpoint(direction));

  while (true) {
    mp->step(direction);
    if (mp->is_at_endpoint(direction)) {
      return breakpoints.end();
    }
    for (auto it = breakpoints.begin(); it != breakpoints.end(); ++it) {
      if (breakpoint_match(mp->get_current_vertex(), *it)) {
        return it;
      }
    }
  }
}

void mdb_templight_be::display_current_frame() const {
  assert(mp && !mp->is_at_start() && !mp->is_finished());
  display_frame(*mp->get_current_edge());
}

void mdb_templight_be::display_backtrace() const {
  const metaprogram::backtrace_t& backtrace = mp->get_backtrace();

  for (unsigned i = 0; i < backtrace.size(); ++i) {
    shell->display(colored_string("#" + std::to_string(i) + " ", color::white));
    display_frame(backtrace[i]);
  }

  shell->display(colored_string(
        "#" + std::to_string(backtrace.size()) + " ", color::white));
  shell->display(highlight_syntax(
        mp->get_vertex_property(mp->get_root_vertex()).name) + "\n");
}

void mdb_templight_be::display_metaprogram_reached_the_beginning() const {
  shell->display("Metaprogram reached the beginning\n");
}

void mdb_templight_be::display_metaprogram_finished() const {
  shell->display(
      "Metaprogram finished\n" +
      highlight_syntax(mp->get_evaluation_result()) + "\n");
}

void mdb_templight_be::display_trace_graph(
    unsigned depth,
    const std::vector<unsigned>& depth_counter,
    bool print_mark) const
{
  assert(depth_counter.size() > depth);

  if (depth > 0) {
    //TODO respect the -H (no syntax highlight parameter)
    for (unsigned i = 1; i < depth; ++i) {
      shell->display(colored_string(
          depth_counter[i] > 0 ? "| " : "  ",
          colors[i % colors.size()]));
    }

    color mark_color = colors[depth % colors.size()];
    if (print_mark) {
      if (depth_counter[depth] == 0) {
        shell->display(colored_string("` ", mark_color));
      } else {
        shell->display(colored_string("+ ", mark_color));
      }
    } else if (depth_counter[depth] > 0) {
      shell->display(colored_string("| ", mark_color));
    } else {
      shell->display("  ");
    }
  }
}

void mdb_templight_be::display_trace_line(
    metaprogram::vertex_descriptor vertex,
    unsigned depth,
    const std::vector<unsigned>& depth_counter,
    const boost::optional<metaprogram::edge_property>& property,
    unsigned width) const
{

  colored_string element_content =
    highlight_syntax(mp->get_vertex_property(vertex).name);

  if (property) {
    element_content += " (" + to_string(property->kind) + ")";
  }

  unsigned non_content_length = 2*depth;

  const unsigned pretty_print_threshold = 10;
  if (width < pretty_print_threshold ||
      non_content_length >= width - pretty_print_threshold)
  {
    // We have no chance to display the graph nicely :(
    display_trace_graph(depth, depth_counter, true);

    shell->display(element_content);
    shell->display("\n");
  } else {
    unsigned content_width = width - non_content_length;
    for (unsigned i = 0; i < element_content.size(); i += content_width) {
      display_trace_graph(depth, depth_counter, i == 0);
      shell->display(element_content, i, content_width);
      shell->display("\n");
    }
  }
}

void mdb_templight_be::display_trace_visit(
    metaprogram::optional_edge_descriptor root_edge,
    boost::optional<unsigned> max_depth,
    metaprogram::discovered_t& discovered,
    unsigned width) const
{

  // -----
  // Customized DFS
  //   The algorithm only checks vertices which are reachable from root_vertex
  // ----

  // This vector counts how many elements are in the to_visit
  // stack for each specific depth.
  // The purpose is to not draw pipes, when a tree element
  // doesn't have any more children.
  // The 0th element is never read.

  const metaprogram::graph_t& graph = mp->get_graph();

  std::vector<unsigned> depth_counter(1);

  typedef std::tuple<
    metaprogram::optional_edge_descriptor,
    unsigned // Depth
  > stack_element;

  // The usual stack for DFS
  std::stack<stack_element> to_visit;

  to_visit.push(std::make_tuple(root_edge, 0));
  ++depth_counter[0]; // This value is neved read

  while (!to_visit.empty()) {
    metaprogram::optional_edge_descriptor edge;
    unsigned depth;
    std::tie(edge, depth) = to_visit.top();
    to_visit.pop();

    --depth_counter[depth];

    metaprogram::vertex_descriptor vertex = [&] {
      if (edge) return mp->get_target(*edge);
      return mp->get_root_vertex();
    }();

    auto property = [&]() -> boost::optional<metaprogram::edge_property> {
      if (edge) return mp->get_edge_property(*edge);
      return boost::none;
    }();

    display_trace_line(vertex, depth, depth_counter, property, width);

    if (discovered[vertex]) {
      continue;
    }
    discovered[vertex] = true;

    if (max_depth && *max_depth <= depth) {
      continue;
    }

    metaprogram::out_edge_iterator begin, end;
    std::tie(begin, end) = boost::out_edges(vertex, graph);

    typedef std::vector<metaprogram::edge_descriptor> edges_t;
    edges_t edges(begin, end);

    if (depth_counter.size() <= depth+1) {
      depth_counter.resize(depth+1+1);
    }

    // Reverse iteration, so types that got instantiated first
    // get on the top of the stack
    for (const metaprogram::edge_descriptor& edge :
        edges | boost::adaptors::reversed)
    {
      if (mp->get_edge_property(edge).enabled) {
        to_visit.push(std::make_tuple(edge, depth+1));

        ++depth_counter[depth+1];
      }
    }
  }
}

void mdb_templight_be::display_current_forwardtrace(
    boost::optional<unsigned> max_depth) const
{
  metaprogram::discovered_t discovered = mp->get_state().discovered;

  display_trace_visit(
      mp->get_current_edge(), max_depth, discovered, shell->width());
}

void mdb_templight_be::display_current_full_forwardtrace(
    boost::optional<unsigned> max_depth) const
{
  metaprogram::discovered_t discovered(mp->get_state().discovered.size());

  display_trace_visit(
      mp->get_current_edge(), max_depth, discovered, shell->width());
}

void mdb_templight_be::display_frame(
    const metaprogram::edge_descriptor& frame) const
{
  shell->display(
      highlight_syntax(
        mp->get_vertex_property(mp->get_target(frame)).name) +
      " (" + to_string(mp->get_edge_property(frame).kind) + ")\n"
  );
}

}
