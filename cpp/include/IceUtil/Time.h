// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    static Time seconds(Int64);
    static Time milliSeconds(Int64);
    static Time microSeconds(Int64);
    
    operator timeval() const;

    Int64 toSeconds() const;
    Int64 toMilliSeconds() const;
    Int64 toMicroSeconds() const;

    std::string toString() const;

    Time operator-() const // Inlined for performance reasons.
    {
	return Time(-_usec);
    }

    Time operator-(const Time& rhs) const // Inlined for performance reasons.
    {
	return Time(_usec - rhs._usec);
    }

    Time operator+(const Time& rhs) const // Inlined for performance reasons.
    {
	return Time(_usec + rhs._usec);
    }

    Time& operator+=(const Time& rhs) // Inlined for performance reasons.
    {
	_usec += rhs._usec;
	return *this;
    }

    Time& operator-=(const Time& rhs) // Inlined for performance reasons.
    {
	_usec -= rhs._usec;
	return *this;
    }

    bool operator<(const Time& rhs) const // Inlined for performance reasons.
    {
	return _usec < rhs._usec;
    }

    bool operator<=(const Time& rhs) const // Inlined for performance reasons.
    {
	return _usec <= rhs._usec;
    }

    bool operator>(const Time& rhs) const // Inlined for performance reasons.
    {
	return _usec > rhs._usec;
    }

    bool operator>=(const Time& rhs) const // Inlined for performance reasons.
    {
	return _usec >= rhs._usec;
    }

    bool operator==(const Time& rhs) const // Inlined for performance reasons.
    {
	return _usec == rhs._usec;
    }

    bool operator!=(const Time& rhs) const // Inlined for performance reasons.
    {
	return _usec != rhs._usec;
    }

    template<typename T>
    Time& operator*=(T rhs)
    {
	_usec *= rhs;
	return *this;
    }

    template<typename T>
    Time& operator/=(T rhs)
    {
	_usec /= rhs;
	return *this;
    }

private:

    Time(Int64);

    Int64 _usec;
};

template<typename T>
Time operator*(const Time& lhs, T rhs)
{
    return Time::microSeconds(static_cast<Int64>(lhs.toMicroSeconds() * rhs));
}

template<typename T>
Time operator*(T rhs, const Time& lhs)
{
    return Time::microSeconds(static_cast<Int64>(lhs * rhs.toMicroSeconds()));
}

template<typename T>
Time operator/(const Time& lhs, T rhs)
{
    return Time::microSeconds(static_cast<Int64>(lhs.toMicroSeconds() / rhs));
}

} // End namespace IceUtil

#endif
