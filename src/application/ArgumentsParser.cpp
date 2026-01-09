#include "ArgumentsParser.h"
#include <iostream>

ArgumentsParser::ArgumentsParser(int argc, char** argv)
    : arguments{}, flags{}, options{}, success{true}, program{} {
    if (argc > 0) {
        program = std::string(argv[0]);
    }

    for (int i = 1; i < argc; ++i) {
        const auto& arg = std::string(argv[i]);
        // std::string::starts_with is C++20, so we implement it manually
        if (arg.size() >= 2 && arg[0] == '-' && arg[1] == '-') {
            auto eqPos = arg.find('=');
            if (eqPos != std::string::npos) {
                auto key = arg.substr(2, eqPos - 2);
                auto value = arg.substr(eqPos + 1);
                if (options.find(key) != options.end()) {
                    std::cerr << "Warning: Option '" << key << "' set multiple times"
                              << std::endl;
                    success = false;
                    return;
                } else {
                    options.emplace(key, value);
                }
            } else {
                auto key = arg.substr(2);
                flags.insert(key);
            }
        } else {
            arguments.push_back(arg);
        }
    }
}

bool ArgumentsParser::isSuccess() const {
    return success;
}

const std::vector<std::string>& ArgumentsParser::getArguments() const {
    return arguments;
}

const std::set<std::string>& ArgumentsParser::getFlags() const {
    return flags;
}

const std::map<std::string, std::string>& ArgumentsParser::getOptions() const {
    return options;
}

const std::string& ArgumentsParser::getProgram() const {
    return program;
}

bool ArgumentsParser::hasFlag(const std::string& flag) const {
    return flags.find(flag) != flags.end();
}

std::string ArgumentsParser::getOption(const std::string& option,
                                             const std::string& defaultValue) const {
    auto it = options.find(option);
    if (it != options.end()) {
        return it->second;
    } else {
        return defaultValue;
    }
}

const std::optional<std::string> ArgumentsParser::getArgument(size_t index) const {
    if (index < arguments.size()) {
        return arguments[index];
    } else {
        return std::nullopt;
    }
}