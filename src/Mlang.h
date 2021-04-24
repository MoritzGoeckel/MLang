#pragma once

#include <string>

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

       public:
        Result(Signal signal, const std::string& content);
        Result(Signal signal);

        operator Signal() const;

        const std::string& getString() const;
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
    Result executeString(std::string theCode);

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
};
