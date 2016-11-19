// Metashell - Interactive C++ template metaprogramming shell
// Copyright (C) 2014, Abel Sinkovics (abel@sinkovics.hu)
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

#include <metashell/command_processor_queue.hpp>
#include <metashell/in_memory_displayer.hpp>
#include <metashell/json_line_reader.hpp>
#include <metashell/null_displayer.hpp>
#include <metashell/null_json_writer.hpp>

#include "mock_command_processor.hpp"
#include "mock_json_writer.hpp"
#include "string_reader.hpp"

#include <gtest/gtest.h>

using namespace metashell;

TEST(json_line_reader, end_of_input)
{
  null_json_writer jw;
  null_displayer d;
  command_processor_queue cpq;
  const line_reader r = build_json_line_reader(string_reader{}, d, jw, cpq);

  ASSERT_TRUE(boost::none == r(">"));
}

TEST(json_line_reader, empty_json)
{
  null_json_writer jw;
  null_displayer d;
  command_processor_queue cpq;
  const line_reader r = build_json_line_reader(string_reader{""}, d, jw, cpq);

  const boost::optional<std::string> l = r(">");

  ASSERT_TRUE(boost::none != l);
  ASSERT_EQ("", *l);
}

TEST(json_line_reader, getting_line)
{
  null_json_writer jw;
  null_displayer d;
  command_processor_queue cpq;
  const line_reader r = build_json_line_reader(
      string_reader{"{\"type\":\"cmd\",\"cmd\":\"int\"}"}, d, jw, cpq);

  const boost::optional<std::string> l = r(">");

  ASSERT_TRUE(boost::none != l);
  ASSERT_EQ("int", *l);
}

TEST(json_line_reader, rejected_json_is_skipped)
{
  null_json_writer jw;
  null_displayer d;
  command_processor_queue cpq;
  const line_reader r = build_json_line_reader(
      string_reader{"\"invalid_json\"", "{\"type\":\"cmd\",\"cmd\":\"int\"}"},
      d, jw, cpq);

  const boost::optional<std::string> l = r(">");

  ASSERT_TRUE(boost::none != l);
  ASSERT_EQ("int", *l);
}

TEST(json_line_reader, command_without_type)
{
  null_json_writer jw;
  in_memory_displayer d;
  command_processor_queue cpq;
  const line_reader r = build_json_line_reader(
      string_reader{"{}", "{\"type\":\"cmd\",\"cmd\":\"int\"}"}, d, jw, cpq);

  const boost::optional<std::string> l = r(">");

  // generates an error
  ASSERT_EQ(std::vector<std::string>{"Command without a type: {}"}, d.errors());

  // skipped
  ASSERT_TRUE(bool(l));
  ASSERT_EQ("int", *l);
}

TEST(json_line_reader, command_of_unknown_type)
{
  null_json_writer jw;
  in_memory_displayer d;
  command_processor_queue cpq;
  const line_reader r = build_json_line_reader(
      string_reader{"{\"type\":\"some unknown type\"}",
                    "{\"type\":\"cmd\",\"cmd\":\"int\"}"},
      d, jw, cpq);

  const boost::optional<std::string> l = r(">");

  // generates an error
  ASSERT_EQ(std::vector<std::string>{"Unknown command type: some unknown type"},
            d.errors());

  // skipped
  ASSERT_TRUE(boost::none != l);
  ASSERT_EQ("int", *l);
}

TEST(json_line_reader, cmd_command_without_cmd_field)
{
  null_json_writer jw;
  in_memory_displayer d;
  command_processor_queue cpq;
  const line_reader r = build_json_line_reader(
      string_reader{"{\"type\":\"cmd\"}", "{\"type\":\"cmd\",\"cmd\":\"int\"}"},
      d, jw, cpq);

  const boost::optional<std::string> l = r(">");

  // generates an error
  ASSERT_EQ(
      std::vector<std::string>{"The cmd field of the cmd command is missing"},
      d.errors());

  // skipped
  ASSERT_TRUE(boost::none != l);
  ASSERT_EQ("int", *l);
}

TEST(json_line_reader, displays_prompt)
{
  mock_json_writer jw;
  null_displayer d;
  command_processor_queue cpq;

  const line_reader r = build_json_line_reader(string_reader{}, d, jw, cpq);

  r(">");

  ASSERT_EQ((std::vector<std::string>{"start_object", "key type",
                                      "string prompt", "key prompt", "string >",
                                      "end_object", "end_document"}),
            jw.calls());
}

TEST(json_line_reader, code_completion_without_code)
{
  null_json_writer jw;
  in_memory_displayer d;
  command_processor_queue cpq;
  const line_reader r = build_json_line_reader(
      string_reader{"{\"type\":\"code_completion\"}",
                    "{\"type\":\"cmd\",\"cmd\":\"int\"}"},
      d, jw, cpq);

  const boost::optional<std::string> l = r(">");

  // generates an error
  ASSERT_EQ(
      std::vector<std::string>{
          "The code field of the code_completion command is missing"},
      d.errors());

  // skipped
  ASSERT_TRUE(boost::none != l);
  ASSERT_EQ("int", *l);
}

TEST(json_line_reader, code_completion_gets_code_completion)
{
  null_json_writer jw;
  null_displayer d;

  bool called = false;
  std::string called_with;

  mock_command_processor* cp = new mock_command_processor;
  cp->code_complete_callback = [&called, &called_with](
      const std::string& code_, std::set<std::string>&) {
    called = true;
    called_with = code_;
  };

  command_processor_queue cpq;
  cpq.push(std::unique_ptr<iface::command_processor>(cp));

  const line_reader r = build_json_line_reader(
      string_reader{"{\"type\":\"code_completion\",\"code\":\"foo\"}"}, d, jw,
      cpq);

  r(">");

  ASSERT_TRUE(called);
  ASSERT_EQ("foo", called_with);
}

TEST(json_line_reader, code_completion_result)
{
  mock_json_writer jw;
  null_displayer d;

  mock_command_processor* cp = new mock_command_processor;
  cp->code_complete_callback = [](
      const std::string&, std::set<std::string>& out_) {
    out_.insert("hello");
    out_.insert("world");
  };

  command_processor_queue cpq;
  cpq.push(std::unique_ptr<iface::command_processor>(cp));

  const line_reader r = build_json_line_reader(
      string_reader{"{\"type\":\"code_completion\",\"code\":\"foo\"}"}, d, jw,
      cpq);

  r(">");

  ASSERT_EQ(
      (std::vector<std::string>{
          "start_object",    "key type",    "string prompt",
          "key prompt",      "string >",    "end_object",
          "end_document",

          "start_object",    "key type",    "string code_completion_result",
          "key completions", "start_array", "string hello",
          "string world",    "end_array",   "end_object",
          "end_document",

          "start_object",    "key type",    "string prompt",
          "key prompt",      "string >",    "end_object",
          "end_document"}),
      jw.calls());
}
