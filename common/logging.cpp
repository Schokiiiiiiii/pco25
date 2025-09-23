#include "logging.h"

void LogFunction(const std::string& str) {
    // write to socket, file, console, e.t.c
#ifndef LOGGING_BE_SILENT
    std::cout << str; //  << std::endl;
#endif
}
