#pragma once

#include <string>

class Mlang {
   public:
    Mlang();

    ~Mlang();

    enum class Signal{
        Failure,
        Success
    };

    struct Settings {
        bool showTokens = false;
        bool showParseTree = false;
        bool showFileContent = false;
        bool showModule = false;
        bool showResult = false;
        bool showPrettyParseTree = false;
        bool showAbastractSyntaxTree = false;
        bool infereTypes = false;
    };

    Settings settings;

    /**
     * Execute mlang source code
     * @param mlang source code
     */
    Signal executeString(std::string theCode);

    /**
     * Loads a source code file and executes it
     * @param path to the file
     * @return nothing
     */
    Signal executeFile(std::string thePath);

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

    // private:
};
