#include "Runner.h"

Runner::Runner() : isBroken(false) {
}

Runner::~Runner() {}

bool Runner::getIsBroken() { return isBroken; }

bool Runner::addModule(const std::string& code) {
    return true;
}

Mlang::Result Runner::run() {
    if (isBroken) {
        return Mlang::Result(Mlang::Result::Signal::Failure)
            .addError("Module invalid");
    }

    return Mlang::Result(Mlang::Result::Signal::Success);
}

