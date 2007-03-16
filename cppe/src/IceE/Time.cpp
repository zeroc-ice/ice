// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DisableWarnings.h>
#include <IceE/Time.h>

#ifndef _WIN32_WCE
#   if defined(_WIN32)
#       include <sys/timeb.h>
#       include <time.h>
#   else
#       include <sys/time.h>
#   endif
#endif

using namespace IceUtil;

IceUtil::Time::Time() :
    _usec(0)
{
}

Time
IceUtil::Time::now()
{
#if defined(_WIN32_WCE)
    //
    // Note that GetTickCount returns the number of ms since the
    // device was started. Time cannot be used to represent an
    // absolute time on CE since GetLocalTime doesn't have millisecond
    // resolution.
    //
    return Time(static_cast<Int64>(GetTickCount()) * 1000);
#elif defined(_WIN32)
    struct _timeb tb;
    _ftime(&tb);
    return Time(tb.time * static_cast<Int64>(1000000) + tb.millitm * static_cast<Int64>(1000));
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    return Time(tv.tv_sec * static_cast<Int64>(1000000) + tv.tv_usec);
#endif
}

Time
IceUtil::Time::seconds(Int64 t)
{
    return Time(t * static_cast<Int64>(1000000));
}

Time
IceUtil::Time::milliSeconds(Int64 t)
{
    return Time(t * static_cast<Int64>(1000));
}

Time
IceUtil::Time::microSeconds(Int64 t)
{
    return Time(t);
}

#ifndef _WIN32
IceUtil::Time::operator timeval() const
{
    timeval tv;
    tv.tv_sec = static_cast<long>(_usec / 1000000);
    tv.tv_usec = static_cast<long>(_usec % 1000000);
    return tv;
}
#endif

Int64
IceUtil::Time::toSeconds() const
{
    return _usec / 1000000;
}

Int64
IceUtil::Time::toMilliSeconds() const
{
    return _usec / 1000;
}

Int64
IceUtil::Time::toMicroSeconds() const
{
    return _usec;
}

double
IceUtil::Time::toSecondsDouble() const
{
    return _usec / 1000000.0;
}

double
IceUtil::Time::toMilliSecondsDouble() const
{
    return _usec / 1000.0;
}

double
IceUtil::Time::toMicroSecondsDouble() const
{
    return static_cast<double>(_usec);
}

Time::Time(Int64 usec) :
    _usec(usec)
{
}
