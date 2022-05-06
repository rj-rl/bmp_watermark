#pragma once

#include <chrono>
#include <iostream>
#include <string>

using namespace std::chrono;

class Log_Duration {
public:
    explicit Log_Duration( const std::string& msg = "" )
        : message( msg + ": " )
        , start( steady_clock::now() )
    {
    }

    ~Log_Duration()
    {
        auto finish = steady_clock::now();
        auto dur = finish - start;
        std::cerr << message
            << duration_cast<milliseconds>(dur).count()
            << " ms" << std::endl;
    }

private:
    std::string message;
    steady_clock::time_point start;
};

#define UNIQ_ID_IMPL(lineno) _a_local_var_##lineno
#define UNIQ_ID(lineno) UNIQ_ID_IMPL(lineno)

#define LOG_DURATION(message)             \
  Log_Duration UNIQ_ID(__LINE__){message};
