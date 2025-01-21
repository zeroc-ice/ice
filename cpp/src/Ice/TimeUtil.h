// Copyright (c) ZeroC, Inc.

#ifndef ICE_TIME_UTIL_H
#define ICE_TIME_UTIL_H

#include "Ice/Config.h"

#include <chrono>
#include <string>

namespace IceInternal
{
    // Convert a time point to a string using the specified format. The format string is a format string for strftime.
    ICE_API std::string timePointToString(const std::chrono::system_clock::time_point& time, const std::string& format);

    // Convert a time point to a string using the format "%x %H:%M:%S". The number of milliseconds is appended to the
    // string.
    ICE_API std::string timePointToDateTimeString(const std::chrono::system_clock::time_point& time);
}

#endif
