// Copyright (c) ZeroC, Inc.

#ifndef _WIN32

#    include "SysLoggerI.h"
#    include "Ice/LocalExceptions.h"

#    include <mutex>
#    include <syslog.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    // openlog/closelog and the syslog ident are process-global, and openlog retains the ident pointer rather than
    // copying it. All SysLoggerI instances therefore share a single openlog call: the first instance donates its
    // prefix as the process-wide ident, and the last instance to be destroyed calls closelog. sysLogMutex guards
    // this state and serializes the syslog calls.
    mutex sysLogMutex;
    int sysLogCount = 0;

    // Intentionally leaked: openlog retains the ident pointer, and a logger stored in a global can still call syslog
    // during static destruction, so this buffer must never be freed.
    string& sysLogIdent = *new string; // NOLINT(cert-err58-cpp)

    int parseFacility(string_view facilityString)
    {
        if (facilityString == "LOG_KERN")
        {
            return LOG_KERN;
        }
        else if (facilityString == "LOG_USER")
        {
            return LOG_USER;
        }
        else if (facilityString == "LOG_MAIL")
        {
            return LOG_MAIL;
        }
        else if (facilityString == "LOG_DAEMON")
        {
            return LOG_DAEMON;
        }
        else if (facilityString == "LOG_AUTH")
        {
            return LOG_AUTH;
        }
        else if (facilityString == "LOG_SYSLOG")
        {
            return LOG_SYSLOG;
        }
        else if (facilityString == "LOG_LPR")
        {
            return LOG_LPR;
        }
        else if (facilityString == "LOG_NEWS")
        {
            return LOG_NEWS;
        }
        else if (facilityString == "LOG_UUCP")
        {
            return LOG_UUCP;
        }
        else if (facilityString == "LOG_CRON")
        {
            return LOG_CRON;
        }
#    ifdef LOG_AUTHPRIV
        else if (facilityString == "LOG_AUTHPRIV")
        {
            return LOG_AUTHPRIV;
        }
#    endif
#    ifdef LOG_FTP
        else if (facilityString == "LOG_FTP")
        {
            return LOG_FTP;
        }
#    endif
        else if (facilityString == "LOG_LOCAL0")
        {
            return LOG_LOCAL0;
        }
        else if (facilityString == "LOG_LOCAL1")
        {
            return LOG_LOCAL1;
        }
        else if (facilityString == "LOG_LOCAL2")
        {
            return LOG_LOCAL2;
        }
        else if (facilityString == "LOG_LOCAL3")
        {
            return LOG_LOCAL3;
        }
        else if (facilityString == "LOG_LOCAL4")
        {
            return LOG_LOCAL4;
        }
        else if (facilityString == "LOG_LOCAL5")
        {
            return LOG_LOCAL5;
        }
        else if (facilityString == "LOG_LOCAL6")
        {
            return LOG_LOCAL6;
        }
        else if (facilityString == "LOG_LOCAL7")
        {
            return LOG_LOCAL7;
        }
        else
        {
            throw InitializationException(
                __FILE__,
                __LINE__,
                "Invalid value for Ice.SyslogFacility: " + string{facilityString});
        }
    }
}

Ice::SysLoggerI::SysLoggerI(string prefix, string_view facilityString)
    : SysLoggerI(std::move(prefix), parseFacility(facilityString))
{
}

Ice::SysLoggerI::SysLoggerI(string prefix, int facility) : _facility(facility), _prefix(std::move(prefix))
{
    lock_guard lock(sysLogMutex);
    if (sysLogCount++ == 0)
    {
        sysLogIdent = _prefix;
        // The facility argument is only a default for syslog calls that don't specify one; each log method passes
        // this logger's facility with the message priority.
        openlog(sysLogIdent.c_str(), LOG_PID | LOG_CONS, 0);
    }
    if (!_prefix.empty() && _prefix != sysLogIdent)
    {
        _bodyPrefix = _prefix + ": ";
    }
}

Ice::SysLoggerI::~SysLoggerI()
{
    lock_guard lock(sysLogMutex);
    if (--sysLogCount == 0)
    {
        closelog();
    }
}

void
Ice::SysLoggerI::print(const string& message)
{
    // The Logger contract requires these methods not to throw, so we let syslog do the formatting instead of
    // building a std::string that could throw bad_alloc.
    lock_guard lock(sysLogMutex);
    syslog(_facility | LOG_INFO, "%s%s", _bodyPrefix.c_str(), message.c_str());
}

void
Ice::SysLoggerI::trace(const string& category, const string& message)
{
    lock_guard lock(sysLogMutex);
    syslog(_facility | LOG_INFO, "%s%s: %s", _bodyPrefix.c_str(), category.c_str(), message.c_str());
}

void
Ice::SysLoggerI::warning(const string& message)
{
    lock_guard lock(sysLogMutex);
    syslog(_facility | LOG_WARNING, "%s%s", _bodyPrefix.c_str(), message.c_str());
}

void
Ice::SysLoggerI::error(const string& message)
{
    lock_guard lock(sysLogMutex);
    syslog(_facility | LOG_ERR, "%s%s", _bodyPrefix.c_str(), message.c_str());
}

string
Ice::SysLoggerI::getPrefix()
{
    return _prefix;
}

Ice::LoggerPtr
Ice::SysLoggerI::cloneWithPrefix(string prefix)
{
    return make_shared<SysLoggerI>(std::move(prefix), _facility);
}

#endif
