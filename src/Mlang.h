#pragma once

#include <string>
#include <vector>

class Mlang {
   public:
    Mlang();

    ~Mlang();

    class Result {
       public:
        enum class Signal { Failure, Success };

       private:
        Signal signal;
        std::string content;
        std::vector<std::string> itsErrors;

       public:
        Result(Signal signal, const std::string& content);
        Result(Signal signal);

        operator Signal() const;

        const std::string& getResult() const;

        Result& addError(const std::string& errorText);

        const std::vector<std::string>& getErrors() const;

        std::string getErrorString() const;
    };

    struct Settings {
        bool showTokens = false;
        bool showFileContent = false;
        bool showResult = false;
        bool showAbastractSyntaxTree = false;
        bool showInferedTypes = false;
        bool showFunctions = false;
        bool showOptimizedModule = false;
    };

    Settings settings;

    /**
     * Execute mlang source code
     * @param mlang source code
     */
    Result executeString(const std::string& theCode);

    /**
     * Loads a source code file and executes it
     * @param path to the file
     * @return nothing
     */
    Result executeFile(std::string thePath);

    /**
     * Initializes Mlang globally
     * Call only once. Call before using any Mlang object
     */
    static void init();

    /**
     * Shutsdown Mlang globally
     * Call only once. Do not use Mlang object after calling shutdwon()
     */
    static void shutdown();

   private:
    Result execute(const std::string& theFile, const std::string& theCode);
};
