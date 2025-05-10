#pragma once

#include <iostream>
#include <memory>

#include "../Mlang.h"
#include "../error/Exceptions.h"

class Runner {
   private:
    bool isBroken;

   public:
    Runner();
    ~Runner();

    bool addModule(const std::string& code);

    bool getIsBroken();
    Mlang::Result run();
};
