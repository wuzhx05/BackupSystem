#pragma once
#ifndef ENV
#define ENV

#include <string>

namespace env {
extern std::string CALLED_TIME;
extern std::string UUID;

void init();
std::string getCurrentTime(const char* format);

}
#endif