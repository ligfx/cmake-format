/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://opensource.org/licenses/BSD-3-Clause for
   details.  */

#pragma once

#include "helpers.h"

struct SwitchOptionDescription {
    std::string option;
    std::string description;
    bool &value_ref;
};

struct ArgumentOptionDescription {
    std::string option;
    std::string value;
    std::string description;
    std::function<void(std::string)> callback;
};

struct opterror_t {
    constexpr opterror_t() {
    }
};
constexpr opterror_t opterror;

static std::function<void(const std::string &)> parse_numeric_option(size_t &ref) {
    return [&](const std::string &value) {
        try {
            ref = std::stoi(value);
        } catch (const std::invalid_argument &) {
            throw opterror;
        }
    };
}

static std::vector<std::string> parse_command_line(int argc, char **argv,
    const std::string &description, const std::vector<SwitchOptionDescription> &switch_options,
    const std::vector<ArgumentOptionDescription> &argument_options) {
    std::vector<std::string> positional_args;
    for (int i = 1; i < argc; i++) {
        const std::string arg{argv[i]};

        if ("-h" == arg || "-help" == arg || "--help" == arg) {
            fprintf(stderr, "usage: %s [options] [file ...]\n", argv[0]);
            fprintf(stderr, "\n");
            fprintf(stderr, "%s\n", description.c_str());
            fprintf(stderr, "\n");
            fprintf(stderr, "options:\n");

            size_t max_option_size = 0;
            for (auto const &p : switch_options) {
                max_option_size = std::max(max_option_size, p.option.size());
            }
            for (auto const &p : argument_options) {
                max_option_size = std::max(max_option_size, p.option.size() + 1 + p.value.size());
            }

            for (auto const &p : argument_options) {
                fprintf(stderr, "  %s=%s%s  %s\n", p.option.c_str(), p.value.c_str(),
                    repeat_string(" ", max_option_size - p.option.size() - 1 - p.value.size())
                        .c_str(),
                    p.description.c_str());
            }
            for (auto const &p : switch_options) {
                fprintf(stderr, "  %s%s  %s\n", p.option.c_str(),
                    repeat_string(" ", max_option_size - p.option.size()).c_str(),
                    p.description.c_str());
            }
            exit(1);
        }

        if (arg[0] != '-') {
            positional_args.emplace_back(arg);
            continue;
        }

        bool handled_arg = false;
        for (auto const &p : switch_options) {
            if (arg == p.option) {
                p.value_ref = true;
                handled_arg = true;
            }
        }
        if (handled_arg) {
            continue;
        }
        for (auto const &p : argument_options) {
            if (arg == p.option) {
                fprintf(stderr, "%s: for the %s option: requires a value!\n", argv[0],
                    p.option.c_str());
                exit(1);
            }
            if (arg.find(p.option + "=") != 0) {
                continue;
            }
            const std::string value = arg.substr(p.option.size() + 1);
            try {
                p.callback(value);
            } catch (const opterror_t &) {
                fprintf(stderr, "%s: for the %s option: '%s' value invalid!\n", argv[0],
                    p.option.c_str(), value.c_str());
                exit(1);
            }
            handled_arg = true;
            break;
        }
        if (handled_arg) {
            continue;
        }
        fprintf(stderr, "%s: unrecognized option '%s'. Try: %s --help\n", argv[0], arg.c_str(),
            argv[0]);
        exit(1);
    }
    return positional_args;
}
