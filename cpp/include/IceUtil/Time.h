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

namespace TimeInternal
{

#ifdef _WIN32
typedef __int64 LongLong;    
#else
typedef long long LongLong;
#endif

} // End namespace TimeInternal

class Time
{
public:

    //
    // Construct a Time object from a timeval.
    //
    Time(const timeval&);

    //
    // Retrieve the current time.
    //
    static Time now();

    //
    // Construct a new Time object from seconds, milli seconds and
    // micro seconds.
    //
    static Time seconds(TimeInternal::LongLong);
    static Time milliSeconds(TimeInternal::LongLong);
    static Time microSeconds(TimeInternal::LongLong);

    //
    // Retrieve the Time as seconds, milli seconds & micro seconds.
    //
    TimeInternal::LongLong seconds() const;
    TimeInternal::LongLong milliSeconds() const;
    TimeInternal::LongLong microSeconds() const;

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
    // Implicit conversion to a timeval.
    //
    operator timeval() const;

private:

    //
    // Private constructor - for use by seconds, milliSeconds &
    // microSeconds.
    //
    Time(TimeInternal::LongLong);

    TimeInternal::LongLong _usec;
};

} // End namespace IceUtil

#endif
