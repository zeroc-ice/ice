// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_UTIL_TIME_H
#define ICE_UTIL_TIME_H

#include <IceUtil/Config.h>

namespace IceUtil
{

class ICE_UTIL_API Time
{
public:

    Time();

    // No copy constructor and assignment operator necessary. The
    // automatically generated copy constructor and assignment
    // operator do the right thing.
    
    static Time now();
    static Time seconds(long);
    static Time milliSeconds(long);
#ifdef _WIN32
    static Time microSeconds(__int64);
#else
    static Time microSeconds(long long);
#endif

    Time operator-() const;
    Time operator-(const Time&) const;
    Time operator+(const Time&) const;
    Time& operator+=(const Time&);
    Time& operator-=(const Time&);

    bool operator<(const Time&) const;
    bool operator<=(const Time&) const;
    bool operator>(const Time&) const;
    bool operator>=(const Time&) const;
    bool operator==(const Time&) const;
    bool operator!=(const Time&) const;

    operator timeval() const;
    operator double() const;

private:

    Time(Int64);

    Int64 _usec;
};

} // End namespace IceUtil

#endif
