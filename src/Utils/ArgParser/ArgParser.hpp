#pragma once

#include <Deps/Deps.hpp>
#include <expected>
#include <format>
#include <optional>
#include <span>
#include <string_view>

namespace Fig::ArgParser
{

    enum class ParseErrorCode
    {
        UnknownOption,
        MissingValue,
        MissingRequired,
        InvalidFormat
    };

    struct ParseError
    {
        ParseErrorCode code;
        String         context;

        String Format() const
        {
            switch (code)
            {
                case ParseErrorCode::UnknownOption:
                    return String(std::format("Unknown option: {}", context.toStdString()));
                case ParseErrorCode::MissingValue:
                    return String(
                        std::format("Missing value for option: {}", context.toStdString()));
                case ParseErrorCode::MissingRequired:
                    return String(
                        std::format("Missing required option: {}", context.toStdString()));
                case ParseErrorCode::InvalidFormat:
                    return String(
                        std::format("Invalid argument format: {}", context.toStdString()));
                default: return String("Unknown parse error");
            }
        }
    };

    class OptionDef
    {
    public:
        OptionDef(char short_name, String long_name) :
            short_name_(short_name), long_name_(std::move(long_name))
        {
        }

        OptionDef &Help(String help_text)
        {
            help_ = std::move(help_text);
            return *this;
        }

        OptionDef &DefaultValue(String value)
        {
            default_value_ = std::move(value);
            return *this;
        }

        OptionDef &Required(bool required = true)
        {
            required_ = required;
            return *this;
        }

        OptionDef &TakesValue(bool takes_value = true)
        {
            takes_value_ = takes_value;
            return *this;
        }

        char                  short_name_;
        String                long_name_;
        String                help_;
        std::optional<String> default_value_;
        bool                  required_    = false;
        bool                  takes_value_ = false;
    };

    class ParseResult
    {
    public:
        bool HasFlag(const String &long_name) const
        {
            auto it = options_.find(long_name);
            return it != options_.end() && it->second == String("true");
        }

        std::optional<String> GetOption(const String &long_name) const
        {
            if (auto it = options_.find(long_name); it != options_.end())
                return it->second;
            return std::nullopt;
        }

        const DynArray<String> &GetPositionals() const
        {
            return positionals_;
        }

    private:
        friend class ArgumentParser;
        HashMap<String, String> options_;
        DynArray<String>        positionals_;
    };

    class ArgumentParser
    {
    public:
        explicit ArgumentParser(String prog_name, String description = String("")) :
            prog_name_(std::move(prog_name)), description_(std::move(description))
        {
        }

        OptionDef &AddFlag(char short_name, const String &long_name)
        {
            return registerOption(short_name, long_name).TakesValue(false);
        }

        OptionDef &AddFlag(const String &long_name)
        {
            return registerOption('\0', long_name).TakesValue(false);
        }

        OptionDef &AddOption(char short_name, const String &long_name)
        {
            return registerOption(short_name, long_name).TakesValue(true);
        }

        OptionDef &AddOption(const String &long_name)
        {
            return registerOption('\0', long_name).TakesValue(true);
        }

        Result<ParseResult, ParseError> Parse(int argc, const char *const *argv) const
        {
            if (argc <= 0)
                return ParseResult{};

            return parseInternal(
                std::span<const char *const>{argv + 1, static_cast<size_t>(argc - 1)})
                .and_then([this](ParseResult res) { return validateRequired(std::move(res)); })
                .and_then([this](ParseResult res) { return applyDefaults(std::move(res)); });
        }

        String FormatHelp() const
        {
            std::string help =
                std::format("Usage: {} [OPTIONS] [ARGS...]\n\n", prog_name_.toStdString());

            if (!description_.empty())
                help += std::format("{}\n\n", description_.toStdString());

            help += "Options:\n";
            for (const auto &opt : options_)
            {
                std::string flags;
                if (opt.short_name_ != '\0')
                    flags = std::format("-{}, --{}", opt.short_name_, opt.long_name_.toStdString());
                else
                    flags = std::format("    --{}", opt.long_name_.toStdString());

                if (opt.takes_value_)
                    flags += " <val>";

                std::string desc = opt.help_.toStdString();
                if (opt.default_value_)
                    desc += std::format(" (default: {})", opt.default_value_->toStdString());
                if (opt.required_)
                    desc += " [required]";

                help += std::format("  {:<25} {}\n", flags, desc);
            }

            return String(help);
        }

    private:
        OptionDef &registerOption(char short_name, const String &long_name)
        {
            size_t idx = options_.size();
            options_.emplace_back(short_name, long_name);
            long_map_[long_name] = idx;
            if (short_name != '\0')
                short_map_[short_name] = idx;
            return options_.back();
        }

        Result<ParseResult, ParseError> parseInternal(std::span<const char *const> args) const
        {
            ParseResult result;
            bool        only_positionals = false;

            for (size_t i = 0; i < args.size(); ++i)
            {
                std::string_view arg{args[i]};

                if (only_positionals)
                {
                    result.positionals_.emplace_back(String(std::string(arg)));
                    continue;
                }

                if (arg == "--")
                {
                    only_positionals = true;
                    continue;
                }

                if (arg.starts_with("--"))
                {
                    auto kv = arg.substr(2);
                    if (kv.empty())
                        return std::unexpected(
                            ParseError{ParseErrorCode::InvalidFormat, String("--")});

                    auto eq_pos = kv.find('=');
                    auto key    = kv.substr(0, eq_pos);
                    auto key_s  = String(std::string(key));

                    auto it = long_map_.find(key_s);
                    if (it == long_map_.end())
                        return std::unexpected(
                            ParseError{ParseErrorCode::UnknownOption, String(std::string(arg))});

                    const auto &def = options_[it->second];

                    if (def.takes_value_)
                    {
                        if (eq_pos != std::string_view::npos)
                        {
                            result.options_[def.long_name_] =
                                String(std::string(kv.substr(eq_pos + 1)));
                        }
                        else
                        {
                            if (i + 1 >= args.size())
                                return std::unexpected(ParseError{
                                    ParseErrorCode::MissingValue, String(std::string(arg))});
                            result.options_[def.long_name_] = String(std::string(args[++i]));
                        }
                    }
                    else
                    {
                        if (eq_pos != std::string_view::npos)
                            return std::unexpected(ParseError{
                                ParseErrorCode::InvalidFormat, String(std::string(arg))});
                        result.options_[def.long_name_] = String("true");
                    }
                }
                else if (arg.starts_with('-') && arg.size() > 1)
                {
                    auto group = arg.substr(1);
                    for (size_t j = 0; j < group.size(); ++j)
                    {
                        char c  = group[j];
                        auto it = short_map_.find(c);
                        if (it == short_map_.end())
                            return std::unexpected(ParseError{
                                ParseErrorCode::UnknownOption, String(std::string(1, c))});

                        const auto &def = options_[it->second];

                        if (def.takes_value_)
                        {
                            if (j + 1 < group.size())
                            {
                                result.options_[def.long_name_] =
                                    String(std::string(group.substr(j + 1)));
                                break;
                            }
                            else
                            {
                                if (i + 1 >= args.size())
                                    return std::unexpected(ParseError{ParseErrorCode::MissingValue,
                                        String(std::format("-{}", c))});
                                result.options_[def.long_name_] = String(std::string(args[++i]));
                            }
                        }
                        else
                        {
                            result.options_[def.long_name_] = String("true");
                        }
                    }
                }
                else
                {
                    result.positionals_.emplace_back(String(std::string(arg)));
                }
            }
            return result;
        }

        Result<ParseResult, ParseError> validateRequired(ParseResult res) const
        {
            for (const auto &opt : options_)
            {
                if (opt.required_ && res.options_.find(opt.long_name_) == res.options_.end())
                {
                    std::string flag =
                        opt.short_name_ != '\0' ?
                            std::format("-{}/--{}", opt.short_name_, opt.long_name_.toStdString()) :
                            std::format("--{}", opt.long_name_.toStdString());
                    return std::unexpected(
                        ParseError{ParseErrorCode::MissingRequired, String(flag)});
                }
            }
            return res;
        }

        Result<ParseResult, ParseError> applyDefaults(ParseResult res) const
        {
            for (const auto &opt : options_)
            {
                if (opt.default_value_ && res.options_.find(opt.long_name_) == res.options_.end())
                {
                    res.options_[opt.long_name_] = *opt.default_value_;
                }
            }
            return res;
        }

        String                  prog_name_;
        String                  description_;
        DynArray<OptionDef>     options_;
        HashMap<String, size_t> long_map_;
        HashMap<char, size_t>   short_map_;
    };

} // namespace Fig::ArgParser