// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_TIME_H
#define ICEE_TIME_H

#include <IceE/Config.h>

namespace IceUtil
{

class ICE_API Time
{
public:

    Time();

    // No copy constructor and assignment operator necessary. The
    // automatically generated copy constructor and assignment
    // operator do the right thing.
    
    static Time now();
    static Time seconds(Int64);
    static Time milliSeconds(Int64);
    static Time microSeconds(Int64);
    
#ifndef _WIN32
    operator timeval() const;
#endif

    Int64 toSeconds() const;
    Int64 toMilliSeconds() const;
    Int64 toMicroSeconds() const;

    double toSecondsDouble() const;
    double toMilliSecondsDouble() const;
    double toMicroSecondsDouble() const;

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
	return (double)_usec / (double)rhs._usec;
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

} // End namespace Ice

#endif
