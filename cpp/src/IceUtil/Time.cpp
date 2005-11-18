// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Time.h>

#ifdef _WIN32
#   include <sys/timeb.h>
#   include <time.h>
#else
#   include <sys/time.h>
#endif

using namespace IceUtil;

Time::Time() :
    _usec(0)
{
}

Time
IceUtil::Time::now()
{
#ifdef _WIN32
    struct _timeb tb;
    _ftime(&tb);
    return Time(static_cast<Int64>(tb.time) * ICE_INT64(1000000) + 
		tb.millitm * 1000);
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    return Time(tv.tv_sec * ICE_INT64(1000000) + tv.tv_usec);
#endif
}

Time
IceUtil::Time::seconds(Int64 t)
{
    return Time(t * ICE_INT64(1000000));
}

Time
IceUtil::Time::milliSeconds(Int64 t)
{
    return Time(t * ICE_INT64(1000));
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

std::string
IceUtil::Time::toString() const
{
    time_t time = static_cast<long>(_usec / 1000000);

    struct tm* t;
#ifdef _WIN32
    t = localtime(&time);
#else
    struct tm tr;
    localtime_r(&time, &tr);
    t = &tr;
#endif

    char buf[32];
    strftime(buf, sizeof(buf), "%x %H:%M:%S", t);

    std::ostringstream os;
    os << buf << ":";
    os.fill('0');
    os.width(3);
    os << static_cast<long>(_usec % 1000000 / 1000);
    return os.str();
}

Time::Time(Int64 usec) :
    _usec(usec)
{
}

std::ostream&
IceUtil::operator<<(std::ostream& out, const Time& tm)
{
    return out << tm.toMicroSeconds() / 1000000.0;
}
