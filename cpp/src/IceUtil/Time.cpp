// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Time.h>

#ifdef _WIN32
#   include <sys/timeb.h>
#else
#   include <sys/time.h>
#endif

using namespace IceUtil;

Time::Time(TimeInternal::LongLong usec) :
    _usec(usec)
{
}

IceUtil::Time::Time(const timeval& tv) :
    _usec((tv.tv_sec * (TimeInternal::LongLong)1000000) + tv.tv_usec)
{
}

Time
IceUtil::Time::now()
{
#ifdef WIN32
    struct _timeb timebuffer;
    _ftime(&timebuffer);

    return Time(timebuffer.time * (TimeInternal::LongLong)1000000) +
               (timebuffer.millitm * (TimeInternal::LongLong)1000);
#else
    struct timeval tv;
    gettimeofday(&tv, 0);

    return Time(tv);
#endif
}

Time
IceUtil::Time::seconds(TimeInternal::LongLong t)
{
    return Time(t * (TimeInternal::LongLong)1000000);
}

Time
IceUtil::Time::milliSeconds(TimeInternal::LongLong t)
{
    return Time(t * (TimeInternal::LongLong)1000);
}

Time
IceUtil::Time::microSeconds(TimeInternal::LongLong t)
{
    return Time(t);
}

Time
IceUtil::Time::operator-() const
{
    return Time(-_usec);
}

Time
IceUtil::Time::operator-(const Time& other) const
{
    return Time(_usec - other._usec);
}

Time
IceUtil::Time::operator+(const Time& other) const
{
    return Time(_usec + other._usec);
}

Time&
IceUtil::Time::operator+=(const Time& other)
{
    _usec += other._usec;
    return *this;
}

Time&
IceUtil::Time::operator-=(const Time& other)
{
    _usec -= other._usec;
    return *this;
}

bool
IceUtil::Time::operator<(const Time& other) const
{
    return _usec < other._usec;
}

bool
IceUtil::Time::operator<=(const Time& other) const
{
    return _usec <= other._usec;
}

bool
IceUtil::Time::operator>(const Time& other) const
{
    return _usec > other._usec;
}

bool
IceUtil::Time::operator>=(const Time& other) const
{
    return _usec >= other._usec;
}

bool
IceUtil::Time::operator==(const Time& other) const
{
    return _usec == other._usec;
}

bool
IceUtil::Time::operator!=(const Time& other) const
{
    return _usec != other._usec;
}

IceUtil::Time::operator timeval() const
{
    timeval tv;
    tv.tv_sec = _usec / 1000000;
    tv.tv_usec = _usec % 1000000;
    return tv;
}

TimeInternal::LongLong
IceUtil::Time::seconds() const
{
    return _usec / 1000000;
}

TimeInternal::LongLong
IceUtil::Time::milliSeconds() const
{
    return _usec / 1000;
}

TimeInternal::LongLong
IceUtil::Time::microSeconds() const
{
    return _usec;
}
