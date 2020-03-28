#pragma once

#include <string>

class Mlang {
   public:
    Mlang();

    ~Mlang();

    struct Settings {
        bool showTokens = false;
        bool showParseTree = false;
        bool showFileContent = false;
        bool showModule = false;
        bool showResult = false;
        bool showPrettyParseTree = false;
    };

    Settings settings;

    /**
     * Execute mlang source code
     * @param mlang source code
     */
    void executeString(std::string theCode);

    /**
     * Loads a source code file and executes it
     * @param path to the file
     * @return nothing
     */
    void executeFile(std::string thePath);

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
