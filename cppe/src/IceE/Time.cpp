// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Time.h>
#include <IceE/SafeStdio.h>

#ifdef _WIN32_WCE
#   include <time.h>
#elif defined(_WIN32)
#   include <sys/timeb.h>
#   include <time.h>
#else
#   include <sys/time.h>
#endif

using namespace IceE;

// XXX: WINCE -- we don't need to support this.
Time::Time() :
    _usec(0)
{
}

Time
IceE::Time::now()
{
#if defined(_WIN32_WCE)
    //
    // Note that GetLocalTime doesn't return milliseconds. The only way to get
    // ms is to use GetTickCount() and that is not time, that is a ms since
    // the device was started.
    //
    SYSTEMTIME t;
    GetLocalTime(&t);
    FILETIME ft;
    SystemTimeToFileTime(&t, &ft);
    //
    // This is hundreds of nanoseconds (ie: 0.1 microsecond).
    //
    Int64 l = (static_cast<ULONGLONG>(ft.dwHighDateTime) << 32) + ft.dwLowDateTime;
    return Time(l/10);
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
IceE::Time::seconds(Int64 t)
{
    return Time(t * static_cast<Int64>(1000000));
}

Time
IceE::Time::milliSeconds(Int64 t)
{
    return Time(t * static_cast<Int64>(1000));
}

Time
IceE::Time::microSeconds(Int64 t)
{
    return Time(t);
}

IceE::Time::operator timeval() const
{
    timeval tv;
    tv.tv_sec = static_cast<long>(_usec / 1000000);
    tv.tv_usec = static_cast<long>(_usec % 1000000);
    return tv;
}

Int64
IceE::Time::toSeconds() const
{
    return _usec / 1000000;
}

Int64
IceE::Time::toMilliSeconds() const
{
    return _usec / 1000;
}

Int64
IceE::Time::toMicroSeconds() const
{
    return _usec;
}

double
IceE::Time::toSecondsDouble() const
{
    return _usec / 1000000.0;
}

double
IceE::Time::toMilliSecondsDouble() const
{
    return _usec / 1000.0;
}

double
IceE::Time::toMicroSecondsDouble() const
{
    return static_cast<double>(_usec);
}

std::string
IceE::Time::toString() const
{
#ifdef _WIN32_WCE
    Int64 l = _usec * 10;
    FILETIME ft;
    ft.dwLowDateTime = static_cast<DWORD>(l & 0xffffffff);
    ft.dwHighDateTime = static_cast<DWORD>(l >> 32);
    SYSTEMTIME t;
    FileTimeToSystemTime(&ft, &t);
    TCHAR date[32];
    TCHAR time[32];
    GetDateFormat(LOCALE_SYSTEM_DEFAULT, LOCALE_NOUSEROVERRIDE, &t, 0, date, sizeof(date));
    GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &t, L"HH':'mm':'ss", time, sizeof(time));

    char buf[32];
    std::string out;
    wcstombs(buf, date, sizeof(buf));
    out += buf;
    out += " ";
    wcstombs(buf, time, sizeof(buf));
    out += buf;

    out += IceE::printfToString(":%03d", static_cast<int>(_usec % 1000000 / 1000));

    return out;
#else
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
    // XXX: safe version of this?
    strftime(buf, sizeof(buf), "%x %H:%M:%S", t);

    std::string out(buf);
    out += IceE::printfToString(":%03d", static_cast<int>(_usec % 1000000 / 1000));
    return out;
#endif
}

Time::Time(Int64 usec) :
    _usec(usec)
{
}
