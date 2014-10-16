
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

#include <metashell/mdb_be_base.hpp>
#include <metashell/mdb_shell.hpp>

#include <boost/assign.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>

namespace metashell {

const mdb_command_handler_map mdb_shell::command_handler =
  mdb_command_handler_map(
    {
      {{"evaluate"}, non_repeatable, &mdb_shell::command_evaluate,
        "[<type>]",
        "Evaluate and start debugging a new metaprogram.",
        "If called with no arguments, then the last evaluated metaprogram will be\n"
        "reevaluated.\n\n"
        "Previous breakpoints are cleared.\n\n"
        "Unlike metashell, evaluate doesn't use metashell::format to avoid cluttering\n"
        "the debugged metaprogram with unrelated code. If you need formatting, you can\n"
        "explicitly enter `metashell::format< <type> >::type` for the same effect."},
      {{"step"}, repeatable, &mdb_shell::command_step,
        "[over] [n]",
        "Step the program.",
        "Argument n means step n times. n defaults to 1 if not specified.\n"
        "Negative n means step the program backwards.\n\n"
        "Use of the `over` qualifier will jump over sub instantiations."},
      {{"rbreak"}, non_repeatable, &mdb_shell::command_rbreak,
        "<regex>",
        "Add breakpoint for all types matching `<regex>`.",
        ""},
      {{"continue"}, repeatable, &mdb_shell::command_continue,
        "[n]",
        "Continue program being debugged.",
        "The program is continued until the nth breakpoint or the end of the program\n"
        "is reached. n defaults to 1 if not specified.\n"
        "Negative n means continue the program backwards."},
      {{"forwardtrace", "ft"}, non_repeatable, &mdb_shell::command_forwardtrace,
        "[full] [n]",
        "Print forwardtrace from the current point.",
        "Use of the full qualifier will expand Memoizations even if that instantiation\n"
        "path has been visited before.\n\n"
        "The n specifier limits the depth of the trace. If n is not specified, then the\n"
        "trace depth is unlimited."},
      {{"backtrace", "bt"}, non_repeatable, &mdb_shell::command_backtrace,
        "",
        "Print backtrace from the current point.",
        ""},
      {{"help"}, non_repeatable, &mdb_shell::command_help,
        "[<command>]",
        "Show help for commands.",
        "If <command> is not specified, show a list of all available commands."},
      {{"quit"} , non_repeatable, &mdb_shell::command_quit,
        "",
        "Quit metadebugger.",
        ""}
    });


mdb_shell::mdb_shell(mdb_be_base& mdb_be) : mdb_be(mdb_be) {
  mdb_be.set_shell(this);
}

void mdb_shell::display(const colored_string& cs) const {
  display(cs, 0, cs.size());
}

std::string mdb_shell::prompt() const {
  return "(mdb) ";
}

bool mdb_shell::stopped() const {
  return is_stopped;
}

void mdb_shell::display_splash() const {
  display_info(
      "For help, type \"help\".\n"
  );
}

void mdb_shell::line_available(const std::string& line_arg) {

  try {
    using boost::algorithm::all;
    using boost::is_space;

    std::string line = line_arg;

    if (line != prev_line && !line.empty()) {
      add_history(line);
    }

    if (line.empty()) {
      if (!last_command_repeatable) {
        return;
      }
      line = prev_line;
    } else {
      prev_line = line;
    }

    if (all(line, is_space())) {
      return;
    }

    auto command_arg_pair = command_handler.get_command_for_line(line);
    if (!command_arg_pair) {
      display_error("Command parsing failed\n");
      last_command_repeatable = false;
      return;
    }

    mdb_command cmd;
    std::string args;
    std::tie(cmd, args) = *command_arg_pair;

    last_command_repeatable = cmd.is_repeatable();

    (this->*cmd.get_func())(args);
  } catch (const std::exception& ex) {
    display_error(std::string("Error: ") + ex.what() + "\n");
  } catch (...) {
    display_error("Unknown error\n");
  }
}

bool mdb_shell::require_empty_args(const std::string& args) const {
  if (!args.empty()) {
    display_error("This command doesn't accept arguments\n");
    return false;
  }
  return true;
}

void mdb_shell::command_continue(const std::string& arg) {

  using boost::spirit::qi::int_;
  using boost::spirit::ascii::space;
  using boost::phoenix::ref;
  using boost::spirit::qi::_1;

  auto begin = arg.begin(),
       end = arg.end();

  int continue_count = 1;

  bool result =
    boost::spirit::qi::phrase_parse(
        begin, end,

        -int_[ref(continue_count) = _1],

        space
    );

  if (!result || begin != end) {
    display_argument_parsing_failed();
    return;
  }

  mdb_be.do_continue(continue_count);
}

void mdb_shell::command_step(const std::string& arg) {
  using boost::spirit::qi::lit;
  using boost::spirit::qi::int_;
  using boost::spirit::ascii::space;
  using boost::phoenix::ref;
  using boost::spirit::qi::_1;

  auto begin = arg.begin(),
       end = arg.end();

  int step_count = 1;
  mdb_be_base::step_type type = mdb_be_base::step_type::normal;

  bool result =
    boost::spirit::qi::phrase_parse(
        begin, end,

        -lit("over")[ref(type) =mdb_be_base::step_type::over] >>
        -int_[ref(step_count) = _1],

        space
    );

  if (!result || begin != end) {
    display_argument_parsing_failed();
    return;
  }

  mdb_be.do_step(type, step_count);
}

void mdb_shell::command_evaluate(const std::string& arg) {
  mdb_be.do_evaluate(arg);
}

void mdb_shell::command_forwardtrace(const std::string& arg) {
  using boost::spirit::qi::lit;
  using boost::spirit::qi::uint_;
  using boost::spirit::ascii::space;
  using boost::spirit::qi::_1;

  namespace phx = boost::phoenix;

  auto begin = arg.begin(),
       end = arg.end();

  mdb_be_base::forwardtrace_type type = mdb_be_base::forwardtrace_type::normal;
  boost::optional<unsigned> max_depth;

  bool result =
    boost::spirit::qi::phrase_parse(
        begin, end,

        -lit("full") [phx::ref(type) = mdb_be_base::forwardtrace_type::full] >>
        -uint_ [phx::ref(max_depth) =_1],

        space
    );

  if (!result || begin != end) {
    display_argument_parsing_failed();
    return;
  }

  mdb_be.do_forwardtrace(type, max_depth);
}

void mdb_shell::command_backtrace(const std::string& arg) {
  if (!require_empty_args(arg)) {
    return;
  }
  mdb_be.do_backtrace();
}

void mdb_shell::command_rbreak(const std::string& arg) {
  mdb_be.do_rbreak(arg);
}

void mdb_shell::command_help(const std::string& arg) {
  if (arg.empty()) {
    display_info(
        "List of available commands:\n\n");
    for (const mdb_command& cmd : command_handler.get_commands()) {
      display_info(
          cmd.get_keys().front() + " -- " +
          cmd.get_short_description() + "\n");
    }
    display_info(
        "\n"
        "Type \"help\" followed by a command name for more information.\n"
        "Command name abbreviations are allowed if unambiguous.\n"
        "A blank line as an input will repeat the last command, if it makes sense.\n");
    return;
  }

  auto command_arg_pair = command_handler.get_command_for_line(arg);
  if (!command_arg_pair) {
    display_error("Command not found\n");
    return;
  }

  using boost::algorithm::join;

  mdb_command cmd;
  std::string command_args;
  std::tie(cmd, command_args) = *command_arg_pair;

  if (!command_args.empty()) {
    display_error("Only one argument expected\n");
    return;
  }

  display_info(
      join(cmd.get_keys(), "|") + " " + cmd.get_usage() + "\n" +
      cmd.get_full_description() + "\n");
}

void mdb_shell::command_quit(const std::string& arg) {
  if (!require_empty_args(arg)) {
    return;
  }
  is_stopped = true;
}

void mdb_shell::display_error(const std::string& str) const {
  display(colored_string(str, color::bright_red));
}

void mdb_shell::display_info(const std::string& str) const {
  display(str);
}

void mdb_shell::display_argument_parsing_failed() const {
  display_error("Argument parsing failed\n");
}

}

