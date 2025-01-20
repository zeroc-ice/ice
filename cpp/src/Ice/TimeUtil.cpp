// Copyright (c) ZeroC, Inc.

#include "TimeUtil.h"

#include <sstream>

using namespace IceInternal;
using namespace std;

string
IceInternal::timePointToString(const chrono::system_clock::time_point& timePoint, const string& format)
{
    time_t time = static_cast<time_t>(chrono::duration_cast<chrono::seconds>(timePoint.time_since_epoch()).count());
    struct tm tr;
#ifdef _MSC_VER
    // localtime_s in Microsoft CRT is incompatible with the C standard as it returns errno_t instead of struct tm*
    errno_t p = localtime_s(&tr, &time);
    if (p)
    {
        return std::string{};
    }
#else
    // the standard localtime_r returns a null pointer on error.
    struct tm* p = localtime_r(&time, &tr);
    if (!p)
    {
        return std::string{};
    }
#endif

    char buf[32];
    return strftime(buf, sizeof(buf), format.c_str(), &tr) == 0 ? std::string{} : std::string{buf};
}

string
IceInternal::timePointToDateTimeString(const chrono::system_clock::time_point& timePoint)
{
    const string format = "%x %H:%M:%S";
    ostringstream os;
    os << timePointToString(timePoint, format);
    os << '.';
    os.fill('0');
    os.width(3);
    auto usec = chrono::duration_cast<chrono::microseconds>(timePoint.time_since_epoch());
    os << static_cast<std::int64_t>(usec.count() % 1000000 / 1000);
    return os.str();
}
