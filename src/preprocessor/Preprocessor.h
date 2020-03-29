#include <string>

class Preprocessor {
   public:
    static void run(std::string& str) {
        // Add block
        str.insert(0, "{");
        str.append("}");
    }
};
