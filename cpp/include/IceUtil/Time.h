// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICEUTIL_TIME_H
#define ICEUTIL_TIME_H

#include <IceUtil/Config.h>

namespace IceUtil
{

class ICE_UTIL_API Time
{
public:

    Time(const timeval&);
    Time();
    
    //
    // Retrieve the current time.
    //
    static Time now();

    //
    // Construct a new Time object from seconds, milli seconds and
    // micro seconds.
    //
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

    //
    // Implicit conversion to a timeval & timespec.
    //
    operator timeval() const;

private:

#ifdef _WIN32
typedef __int64 LongLong;    
#else
typedef long long LongLong;
#endif

    //
    // Private constructor.
    //
    Time(LongLong);

    LongLong _usec;
};

} // End namespace IceUtil

#endif
