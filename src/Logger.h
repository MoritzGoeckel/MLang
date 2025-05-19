#pragma once

#include <iostream>

#define LOG std::cout << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << ": "

#define STR(x) #x << "=" << x << " "
#define ENDL std::endl