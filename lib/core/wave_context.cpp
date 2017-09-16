// Metashell - Interactive C++ template metaprogramming shell
// Copyright (C) 2017, Abel Sinkovics (abel@sinkovics.hu)
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

#include <metashell/wave_context.hpp>

#include <sstream>
#include <stdexcept>

namespace
{
  void apply(metashell::wave_context& ctx_,
             const metashell::data::includes& includes_)
  {
    for (const boost::filesystem::path& p : includes_.sys)
    {
      ctx_.add_sysinclude_path(p.string().c_str());
    }
    for (const boost::filesystem::path& p : includes_.quote)
    {
      ctx_.add_include_path(p.string().c_str());
      ctx_.add_sysinclude_path(p.string().c_str());
    }
  }

  boost::wave::language_support apply(boost::wave::language_support lng_,
                                      const metashell::data::wave_config& cfg_)
  {
    if (cfg_.standard)
    {
      switch (*cfg_.standard)
      {
      case metashell::data::wave_standard::c99:
        lng_ = boost::wave::language_support(
            boost::wave::support_c99 |
            boost::wave::support_option_convert_trigraphs |
            boost::wave::support_option_emit_line_directives |
            boost::wave::support_option_include_guard_detection |
            boost::wave::support_option_emit_pragma_directives |
            boost::wave::support_option_insert_whitespace |
            boost::wave::support_option_no_newline_at_end_of_file);
        break;
      case metashell::data::wave_standard::cpp11:
        lng_ = boost::wave::language_support(
            boost::wave::support_cpp0x |
            boost::wave::support_option_convert_trigraphs |
            boost::wave::support_option_long_long |
            boost::wave::support_option_emit_line_directives |
            boost::wave::support_option_include_guard_detection |
            boost::wave::support_option_emit_pragma_directives |
            boost::wave::support_option_insert_whitespace |
            boost::wave::support_option_no_newline_at_end_of_file);
        break;
      }
    }
    if (cfg_.long_long)
    {
      lng_ = boost::wave::enable_long_long(lng_);
    }
    if (cfg_.variadics)
    {
      lng_ = boost::wave::enable_variadics(lng_);
    }
    return lng_;
  }
}

namespace metashell
{
  void apply(wave_context& ctx_, const data::wave_config& cfg_)
  {
    ::apply(ctx_, cfg_.includes);
    ctx_.set_language(::apply(ctx_.get_language(), cfg_));

    for (const std::string& macro : cfg_.macros)
    {
      ctx_.add_macro_definition(macro);
    }
  }

  std::string to_string(const boost::wave::cpp_exception& error_)
  {
    std::ostringstream result;
    result << error_.file_name() << ":" << error_.line_no() << ":"
           << error_.column_no() << ": " << error_.description();
    return result.str();
  }

  void preprocess(wave_context& ctx_)
  {
    try
    {
      const auto e = ctx_.end();
      for (auto i = ctx_.begin(); i != e; ++i)
        ;
    }
    catch (const boost::wave::cpp_exception& error_)
    {
      throw std::runtime_error(metashell::to_string(error_));
    }
  }
}
