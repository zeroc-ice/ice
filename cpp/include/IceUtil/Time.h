// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
    operator float() const;
    operator double() const;

private:

#ifdef _WIN32
typedef __int64 LongLong;    
#else
typedef long long LongLong;
#endif

    Time(LongLong);

    LongLong _usec;
};

} // End namespace IceUtil

#endif
