#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <optional>

class ArgumentsParser {
public:
    ArgumentsParser(int argc, char** argv);

    bool isSuccess() const;
    const std::vector<std::string>& getArguments() const;
    const std::set<std::string>& getFlags() const;
    const std::map<std::string, std::string>& getOptions() const;
    const std::string& getProgram() const;

    bool hasFlag(const std::string& flag) const;
    std::string getOption(const std::string& option,
                                const std::string& defaultValue = "") const;
    const std::optional<std::string> getArgument(size_t index) const;
private:
    std::vector<std::string> arguments;
    std::set<std::string> flags;
    std::map<std::string, std::string> options;
    bool success;
    std::string program;
};

#endif // ARGUMENT_PARSER_H
