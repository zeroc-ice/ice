// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_TIME_H
#define ICE_UTIL_TIME_H

#include <IceUtil/Config.h>

#ifndef _WIN32
#   include <sys/time.h>
#endif

namespace IceUtil
{

class ICE_UTIL_API Time
{
public:

    Time();

    // No copy constructor and assignment operator necessary. The
    // automatically generated copy constructor and assignment
    // operator do the right thing.

    enum Clock { Realtime, Monotonic };
    static Time now(Clock = Realtime);

    static Time seconds(Int64);
    static Time milliSeconds(Int64);
    static Time microSeconds(Int64);

    static Time secondsDouble(double);
    static Time milliSecondsDouble(double);
    static Time microSecondsDouble(double);

#ifndef _WIN32
    operator timeval() const;
#endif

    Int64 toSeconds() const;
    Int64 toMilliSeconds() const;
    Int64 toMicroSeconds() const;

    double toSecondsDouble() const;
    double toMilliSecondsDouble() const;
    double toMicroSecondsDouble() const;

    std::string toDateTime() const;
    std::string toDuration() const;
    std::string toString(const std::string&) const;

    Time operator-() const
    {
        return Time(-_usec);
    }

    Time operator-(const Time& rhs) const
    {
        return Time(_usec - rhs._usec);
    }

    Time operator+(const Time& rhs) const
    {
        return Time(_usec + rhs._usec);
    }

    Time& operator+=(const Time& rhs)
    {
        _usec += rhs._usec;
        return *this;
    }

    Time& operator-=(const Time& rhs)
    {
        _usec -= rhs._usec;
        return *this;
    }

    bool operator<(const Time& rhs) const
    {
        return _usec < rhs._usec;
    }

    bool operator<=(const Time& rhs) const
    {
        return _usec <= rhs._usec;
    }

    bool operator>(const Time& rhs) const
    {
        return _usec > rhs._usec;
    }

    bool operator>=(const Time& rhs) const
    {
        return _usec >= rhs._usec;
    }

    bool operator==(const Time& rhs) const
    {
        return _usec == rhs._usec;
    }

    bool operator!=(const Time& rhs) const
    {
        return _usec != rhs._usec;
    }

    double operator/(const Time& rhs) const
    {
        return static_cast<double>(_usec) / static_cast<double>(rhs._usec);
    }

    Time& operator*=(int rhs)
    {
        _usec *= rhs;
        return *this;
    }

    Time operator*(int rhs) const
    {
        Time t;
        t._usec = _usec * rhs;
        return t;
    }

    Time& operator/=(int rhs)
    {
        _usec /= rhs;
        return *this;
    }

    Time operator/(int rhs) const
    {
        Time t;
        t._usec = _usec / rhs;
        return t;
    }

    Time& operator*=(Int64 rhs)
    {
        _usec *= rhs;
        return *this;
    }

    Time operator*(Int64 rhs) const
    {
        Time t;
        t._usec = _usec * rhs;
        return t;
    }

    Time& operator/=(Int64 rhs)
    {
        _usec /= rhs;
        return *this;
    }

    Time operator/(Int64 rhs) const
    {
        Time t;
        t._usec = _usec / rhs;
        return t;
    }

    Time& operator*=(double rhs)
    {
        _usec = static_cast<Int64>(static_cast<double>(_usec) * rhs);
        return *this;
    }

    Time operator*(double rhs) const
    {
        Time t;
        t._usec = static_cast<Int64>(static_cast<double>(_usec) * rhs);
        return t;
    }

    Time& operator/=(double rhs)
    {
        _usec = static_cast<Int64>(static_cast<double>(_usec) / rhs);
        return *this;
    }

    Time operator/(double rhs) const
    {
        Time t;
        t._usec = static_cast<Int64>(static_cast<double>(_usec) / rhs);
        return t;
    }

private:

    Time(Int64);

    Int64 _usec;
};

ICE_UTIL_API std::ostream& operator<<(std::ostream&, const Time&);

} // End namespace IceUtil

#endif
