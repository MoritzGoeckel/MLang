#pragma once

#include <iostream>
#include <memory>

#include "../core/Mlang.h"
#include "../error/Exceptions.h"

class Runner {
   private:
    bool isBroken;

   public:
    Runner();
    ~Runner();

    bool addModule(const std::string& code);

    bool getIsBroken();
    core::Mlang::Result run();
};
