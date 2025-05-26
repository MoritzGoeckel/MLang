#include "Runner.h"

Runner::Runner() : isBroken(false) {
}

Runner::~Runner() {}

bool Runner::getIsBroken() { return isBroken; }

bool Runner::addModule(const std::string& code) {
    return true;
}

core::Mlang::Result Runner::run() {
    if (isBroken) {
        return core::Mlang::Result(core::Mlang::Result::Signal::Failure)
            .addError("Module invalid");
    }

    return core::Mlang::Result(core::Mlang::Result::Signal::Success);
}

