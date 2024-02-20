//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_TIME_UTIL_H
#define ICE_TIME_UTIL_H

#include <Ice/Config.h>

#include <chrono>
#include <string>

namespace IceInternal
{

ICE_API std::string timePointToString(
    const std::chrono::system_clock::time_point&,
    const std::string&);

ICE_API std::string timePointToDateTimeString(const std::chrono::system_clock::time_point&);

}

#endif
