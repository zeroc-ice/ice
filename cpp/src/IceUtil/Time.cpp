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

Time::Time() :
    _usec(0)
{
}

IceUtil::Time::Time(const timeval& tv) :
    _usec((tv.tv_sec * (LongLong)1000000) + tv.tv_usec)
{
}

Time
IceUtil::Time::now()
{
#ifdef WIN32
    struct _timeb timebuffer;
    _ftime(&timebuffer);

    return Time(timebuffer.time * (LongLong)1000000) +
               (timebuffer.millitm * (LongLong)1000);
#else
    struct timeval tv;
    gettimeofday(&tv, 0);

    return Time(tv);
#endif
}

Time
IceUtil::Time::seconds(long t)
{
    return Time(t * (LongLong)1000000);
}

Time
IceUtil::Time::milliSeconds(long t)
{
    return Time(t * (LongLong)1000);
}

#ifdef _WIN32
Time
IceUtil::Time::microSeconds(__int64 t)
{
    return Time(t);
}
#else
Time
IceUtil::Time::microSeconds(long long t)
{
    return Time(t);
}
#endif

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

//
// Private constructor.
//
Time::Time(LongLong usec) :
    _usec(usec)
{
}
