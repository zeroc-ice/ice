// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Exception.h>
#include <IceUtil/Time.h>
#include <iomanip>

#ifdef _WIN32
#   include <sys/timeb.h>
#   include <time.h>
#else
#   include <sys/time.h>
#endif

#ifdef __APPLE__
#   include <mach/mach.h>
#   include <mach/mach_time.h>
#endif

using namespace IceUtil;

#ifdef _WIN32

namespace
{

double frequency = -1.0;

//
// Initialize the frequency
//
class InitializeFrequency
{
public:

    InitializeFrequency()
    {
        //
        // Get the frequency of performance counters. We also make a call to
        // QueryPerformanceCounter to ensure it works. If it fails or if the
        // call to QueryPerformanceFrequency fails, the frequency will remain
        // set to -1.0 and ftime will be used instead.
        //
        Int64 v;
        if(QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&v)))
        {
            if(QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&v)))
            {
                frequency = static_cast<double>(v);
            }
        }
    }
};
InitializeFrequency frequencyInitializer;

}
#endif

#if defined(__APPLE__)
namespace
{

double machMultiplier = 1.0;
class InitializeTime
{
public:

    InitializeTime()
    {
        mach_timebase_info_data_t initTimeBase = { 0, 0 };
        mach_timebase_info(&initTimeBase);
        machMultiplier = static_cast<double>(initTimeBase.numer) / initTimeBase.denom / ICE_INT64(1000);
    }
};
InitializeTime initializeTime;

}
#endif

Time::Time() :
    _usec(0)
{
}

Time
IceUtil::Time::now(Clock clock)
{
    if(clock == Realtime)
    {
#ifdef _WIN32
#  if defined(_MSC_VER)
        struct _timeb tb;
        _ftime(&tb);
#  elif defined(__MINGW32__)
        struct timeb tb;
        ftime(&tb);
#  endif
        return Time(static_cast<Int64>(tb.time) * ICE_INT64(1000000) + tb.millitm * 1000);
#else
        struct timeval tv;
        if(gettimeofday(&tv, 0) < 0)
        {
            assert(0);
            throw SyscallException(__FILE__, __LINE__, errno);
        }
        return Time(tv.tv_sec * ICE_INT64(1000000) + tv.tv_usec);
#endif
    }
    else // Monotonic
    {
#if defined(_WIN32)
        if(frequency > 0.0)
        {
            Int64 count;
            if(!QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&count)))
            {
                assert(0);
                throw SyscallException(__FILE__, __LINE__, GetLastError());
            }
            return Time(static_cast<Int64>(count / frequency * 1000000.0));
        }
        else
        {
#  if defined(_MSC_VER)
            struct _timeb tb;
            _ftime(&tb);
#  elif defined(__MINGW32__)
            struct timeb tb;
            ftime(&tb);
#  endif
            return Time(static_cast<Int64>(tb.time) * ICE_INT64(1000000) + tb.millitm * 1000);
        }
#elif defined(__hppa)
        //
        // HP does not support CLOCK_MONOTONIC
        //
        struct timeval tv;
        if(gettimeofday(&tv, 0) < 0)
        {
            assert(0);
            throw SyscallException(__FILE__, __LINE__, errno);
        }
        return Time(tv.tv_sec * ICE_INT64(1000000) + tv.tv_usec);
#elif defined(__APPLE__)
       return Time(mach_absolute_time() * machMultiplier);
#else
        struct timespec ts;
        if(clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
        {
            assert(0);
            throw SyscallException(__FILE__, __LINE__, errno);
        }
        return Time(ts.tv_sec * ICE_INT64(1000000) + ts.tv_nsec / ICE_INT64(1000));
#endif
    }
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

Time
IceUtil::Time::secondsDouble(double t)
{
    return Time(Int64(t * 1000000));
}

Time
IceUtil::Time::milliSecondsDouble(double t)
{
    return Time(Int64(t * 1000));
}

Time
IceUtil::Time::microSecondsDouble(double t)
{
    return Time(Int64(t));
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
IceUtil::Time::toDateTime() const
{
    std::ostringstream os;
    os << toString("%x %H:%M:%S") << ".";
    os.fill('0');
    os.width(3);
    os << static_cast<long>(_usec % 1000000 / 1000);
    return os.str();
}

std::string
IceUtil::Time::toDuration() const
{
    Int64 usecs = _usec % 1000000;
    Int64 secs = _usec / 1000000 % 60;
    Int64 mins = _usec / 1000000 / 60 % 60;
    Int64 hours = _usec / 1000000 / 60 / 60 % 24;
    Int64 days = _usec / 1000000 / 60 / 60 / 24;

    using namespace std;

    ostringstream os;
    if(days != 0)
    {
        os << days << "d ";
    }
    os << setfill('0') << setw(2) << hours << ":" << setw(2) << mins << ":" << setw(2) << secs;
    if(usecs != 0)
    {
        os << "." << setw(3) << (usecs / 1000);
    }

    return os.str();
}

std::string
IceUtil::Time::toString(const std::string& format) const
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
    if(strftime(buf, sizeof(buf), format.c_str(), t) == 0)
    {
        return std::string();
    }
    return std::string(buf);
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
