// **********************************************************************
//
// Copyright (c) 2003
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
#ifdef WIN32
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

Time
IceUtil::Time::operator-() const
{
    return Time(-_usec);
}

Time
IceUtil::Time::operator-(const Time& rhs) const
{
    return Time(_usec - rhs._usec);
}

Time
IceUtil::Time::operator+(const Time& rhs) const
{
    return Time(_usec + rhs._usec);
}

Time&
IceUtil::Time::operator+=(const Time& rhs)
{
    _usec += rhs._usec;
    return *this;
}

Time&
IceUtil::Time::operator-=(const Time& rhs)
{
    _usec -= rhs._usec;
    return *this;
}

bool
IceUtil::Time::operator<(const Time& rhs) const
{
    return _usec < rhs._usec;
}

bool
IceUtil::Time::operator<=(const Time& rhs) const
{
    return _usec <= rhs._usec;
}

bool
IceUtil::Time::operator>(const Time& rhs) const
{
    return _usec > rhs._usec;
}

bool
IceUtil::Time::operator>=(const Time& rhs) const
{
    return _usec >= rhs._usec;
}

bool
IceUtil::Time::operator==(const Time& rhs) const
{
    return _usec == rhs._usec;
}

bool
IceUtil::Time::operator!=(const Time& rhs) const
{
    return _usec != rhs._usec;
}

IceUtil::Time::operator timeval() const
{
    timeval tv;
    tv.tv_sec = static_cast<long>(_usec / 1000000);
    tv.tv_usec = static_cast<long>(_usec % 1000000);
    return tv;
}

IceUtil::Time::operator double() const
{
    return _usec / 1000000.0L;
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
