
/*

    log.h

    write output into a log file


*/


#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>

// osabs is responsible for calling init() and deinit()
// in order to have the log available most of the time,
// - init() should be called as soon as possible
// - deinit() should be called as late as possible

// Log::code() logs a block of text with line numbers
// Log::log() can be used as an output stream, e.g. Log::log() << "hello world"

class Log
{
    public:

    static std::ofstream& log();

    static void code(const char *text);

    static void init();
    static void deinit();
};

#endif

