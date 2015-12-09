// Metashell - Interactive C++ template metaprogramming shell
// Copyright (C) 2015, Abel Sinkovics (abel@sinkovics.hu)
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

#include <metashell/engine_clang.hpp>

#include <metashell/data/command.hpp>
#include <metashell/exception.hpp>
#include <metashell/for_each_line.hpp>
#include <metashell/source_position.hpp>
#include <metashell/unsaved_file.hpp>
#include <metashell/metashell.hpp>
#include <metashell/clang_binary.hpp>

#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <fstream>
#include <memory>

using namespace metashell;

namespace
{
  data::process_output run_clang(
    iface::executable& clang_binary_,
    std::vector<std::string> clang_args_,
    const std::string& input_
  )
  {
    clang_args_.push_back("-"); //Compile from stdin

    return clang_binary_.run(clang_args_, input_);
  }

  std::pair<std::string, std::string> find_completion_start(
    const std::string& s_
  )
  {
    typedef std::pair<std::string, std::string> string_pair;

    const data::command cmd(s_);

    std::ostringstream o;
    data::token last_token;
    bool first = true;
    for (auto i = cmd.begin(), e = cmd.end(); i != e; ++i)
    {
      if (first)
      {
        first = false;
      }
      else
      {
        o << last_token.value();
      }
      last_token = *i;
    }

    if (first) // no token
    {
      return string_pair("", "");
    }
    else
    {
      if (
        last_token.category() == data::token_category::identifier
        || last_token.category() == data::token_category::keyword
      )
      {
        return string_pair(o.str(), last_token.value());
      }
      else
      {
        o << last_token.value();
        return string_pair(o.str(), "");
      }
    }
  }

  boost::optional<std::string> remove_prefix(
    const std::string& s_,
    const std::string& prefix_
  )
  {
    if (boost::starts_with(s_, prefix_))
    {
      return std::string(s_.begin() + prefix_.length(), s_.end());
    }
    else
    {
      return boost::none;
    }
  }

  boost::optional<std::string> parse_completion(std::string line_)
  {
    if (const auto without_completion = remove_prefix(line_, "COMPLETION: "))
    {
      if (const auto pattern = remove_prefix(*without_completion, "Pattern : "))
      {
        const auto prefix_end = pattern->find("<#");
        return
          std::string(
            pattern->begin(),
            prefix_end == std::string::npos ?
              pattern->end() :
              pattern->begin() + prefix_end
          );
      }
      else
      {
        return
          std::string(
            without_completion->begin(),
            std::find(
              without_completion->begin(),
              without_completion->end(),
              ' '
            )
          );
      }
    }
    else
    {
      return boost::none;
    }
  }

  std::vector<std::string> clang_args(
    const std::string& internal_dir_,
    const std::vector<std::string>& extra_args_
  )
  {
    std::vector<std::string>
      args{
        "-Wfatal-errors",
        "-iquote", ".",
        "-x", "c++-header",
        "-I", internal_dir_
      };

    args.insert(args.end(), extra_args_.begin(), extra_args_.end());

    return args;
  }

  class engine_clang : public iface::engine
  {
  public:
    engine_clang(
      const std::string& clang_path_,
      const std::string& internal_dir_,
      const std::string& env_path_,
      const std::vector<std::string>& extra_args_,
      logger* logger_
    ) :
      _clang_binary(
        clang_path_,
        clang_args(internal_dir_, extra_args_),
        logger_
      ),
      _internal_dir(internal_dir_),
      _env_path(env_path_),
      _logger(logger_)
    {}

    virtual data::result eval_tmp_formatted(
      const iface::environment& env_,
      const std::string& tmp_exp_,
      bool use_precompiled_headers_
    ) override
    {
      using std::string;
      using std::pair;
    
      METASHELL_LOG(
        _logger,
        "Checking if metaprogram can be evaluated without metashell::format: "
        + tmp_exp_
      );
    
      const data::result
        simple = eval(env_, tmp_exp_, boost::none, use_precompiled_headers_);
    
      METASHELL_LOG(
        _logger,
        !simple.successful ?
          "Errors occured during metaprogram evaluation. Displaying errors"
          " coming from the metaprogram without metashell::format" :
          "No errors occured during metaprogram evaluation. Re-evaluating it"
          " with metashell::format"
      );
    
      return
        simple.successful ?
          eval(
            env_,
            "::metashell::format<" + tmp_exp_ + ">::type",
            boost::none,
            use_precompiled_headers_
          ) :
          simple;
    }

    virtual data::result eval(
      const iface::environment& env_,
      const boost::optional<std::string>& tmp_exp_,
      const boost::optional<std::string>& templight_dump_path_,
      bool use_precompiled_headers_
    ) override
    {
      std::vector<std::string> clang_args{"-Xclang", "-ast-dump"};
      if (use_precompiled_headers_)
      {
        clang_args.push_back("-include");
        clang_args.push_back(_env_path);
      }
      if (templight_dump_path_)
      {
        clang_args.push_back("-Xtemplight");
        clang_args.push_back("-profiler");
        clang_args.push_back("-Xtemplight");
        clang_args.push_back("-safe-mode");
      
        // templight can't be forced to generate output file with
        // -Xtemplight -output=<file> for some reason
        // A workaround is to specify a standard output location with -o
        // then append ".trace.pbf" to the specified file (on the calling side)
        clang_args.push_back("-o");
        clang_args.push_back(*templight_dump_path_);
      }

      const data::process_output output =
        run_clang(
          _clang_binary,
          clang_args,
          tmp_exp_ ?
            env_.get_appended(
              "::metashell::impl::wrap< " + *tmp_exp_ + " > __metashell_v;\n"
            ) :
            env_.get()
        );
    
      const bool success = output.exit_code() == data::exit_code_t(0);
    
      return
        data::result{
          success,
          success && tmp_exp_ ?
            get_type_from_ast_string(output.standard_output()) :
            "",
          success ? "" : output.standard_error(),
          ""
        };
    }

    virtual data::result validate_code(
      const std::string& src_,
      const data::config& config_,
      const iface::environment& env_,
      bool use_precompiled_headers_
    ) override
    {
      METASHELL_LOG(_logger, "Validating code " + src_);
    
      try
      {
        const std::string src = env_.get_appended(src_);
        std::vector<std::string> clang_args;
        if (use_precompiled_headers_)
        {
          clang_args.push_back("-include");
          clang_args.push_back(_env_path);
        }
    
        const data::process_output
          output = run_clang(_clang_binary, clang_args, src);
    
        const bool accept =
          output.exit_code() == data::exit_code_t(0)
          && output.standard_error().empty();
    
        return
          data::result{
            accept,
            "",
            output.standard_error(),
            accept && config_.verbose ? src : ""
          };
      }
      catch (const std::exception& e)
      {
        return data::result(false, "", e.what(), "");
      }
    }

    virtual void code_complete(
      const iface::environment& env_,
      const std::string& src_,
      std::set<std::string>& out_,
      bool use_precompiled_headers_
    ) override
    {
      using boost::starts_with;
    
      using std::pair;
      using std::string;
      using std::set;
    
      METASHELL_LOG(_logger, "Code completion of " + src_);
    
      const pair<string, string> completion_start = find_completion_start(src_);
    
      METASHELL_LOG(
        _logger,
        "Part kept for code completion: " + completion_start.first
      );
    
      const data::unsaved_file src(
        env_.internal_dir() + "/code_complete.cpp",
        env_.get_appended(completion_start.first)
      );
    
      generate(src);
    
      const source_position sp = source_position_of(src.content());
    
      std::vector<std::string>
        clang_args{
          "-fsyntax-only",
          "-Xclang",
          "-code-completion-at=" + src.filename() + ":" + to_string(sp),
          src.filename()
        };
     
      if (use_precompiled_headers_)
      {
        clang_args.push_back("-include");
        clang_args.push_back(_env_path);
      }
   
      const data::process_output o = _clang_binary.run(clang_args, "");
    
      METASHELL_LOG(_logger, "Exit code of clang: " + to_string(o.exit_code()));
    
      const std::string out = o.standard_output();
      out_.clear();
      const int prefix_len = completion_start.second.length();
      for_each_line(
        out,
        [&out_, &completion_start, prefix_len](const std::string& line_)
        {
          if (const boost::optional<std::string> comp = parse_completion(line_))
          {
            if (
              starts_with(*comp, completion_start.second)
              && *comp != completion_start.second
            )
            {
              out_.insert(string(comp->begin() + prefix_len, comp->end()));
            }
          }
        }
      );
    }

    virtual void precompile(const std::string& fn_) override
    {
      using boost::algorithm::trim_copy;

      METASHELL_LOG(_logger, "Generating percompiled header for " + fn_);

      std::vector<std::string>
        args{
          "-iquote", ".",
          "-w",
          "-o", fn_ + ".pch",
          fn_
        };

      const data::process_output o = _clang_binary.run(args, "");
      const std::string err = o.standard_output() + o.standard_error();
      if (
        !err.empty()
        // clang displays this even when "-w" is used. This can be ignored
        && trim_copy(err) !=
          "warning: precompiled header used __DATE__ or __TIME__."
      )
      {
        throw exception("Error precompiling header " + fn_ + ": " + err);
      }
    }

  private:
    clang_binary _clang_binary;
    std::string _internal_dir;
    std::string _env_path;
    logger* _logger;
  };

} // anonymous namespace

std::unique_ptr<iface::engine> metashell::create_clang_engine(
  const std::string& clang_path_,
  const std::string& internal_dir_,
  const std::string& env_filename_,
  const std::vector<std::string>& extra_args_,
  logger* logger_
)
{
  return
    std::unique_ptr<iface::engine>(
      new engine_clang(
        clang_path_,
        internal_dir_,
        internal_dir_ + "/" + env_filename_,
        extra_args_,
        logger_
      )
    );
}

