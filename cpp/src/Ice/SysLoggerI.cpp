// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/SysLoggerI.h>
#include <Ice/LocalException.h>
#include <syslog.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

Ice::SysLoggerI::SysLoggerI(const string& prefix, const string& facilityString) :
    _facility(0),
    _prefix(prefix)
{
    if(facilityString == "LOG_KERN")
    {
        _facility = LOG_KERN;
    }
    else if(facilityString == "LOG_USER")
    {
        _facility = LOG_USER;
    }
    else if(facilityString == "LOG_MAIL")
    {
        _facility = LOG_MAIL;
    }
    else if(facilityString == "LOG_DAEMON")
    {
        _facility = LOG_DAEMON;
    }
    else if(facilityString == "LOG_AUTH")
    {
        _facility = LOG_AUTH;
    }
    else if(facilityString == "LOG_SYSLOG")
    {
        _facility = LOG_SYSLOG;
    }
    else if(facilityString == "LOG_LPR")
    {
        _facility = LOG_LPR;
    }
    else if(facilityString == "LOG_NEWS")
    {
        _facility = LOG_NEWS;
    }
    else if(facilityString == "LOG_UUCP")
    {
        _facility = LOG_UUCP;
    }
    else if(facilityString == "LOG_CRON")
    {
        _facility = LOG_CRON;
    }
#ifdef LOG_AUTHPRIV
    else if(facilityString == "LOG_AUTHPRIV")
    {
        _facility = LOG_AUTHPRIV;
    }
#endif
#ifdef LOG_FTP
    else if(facilityString == "LOG_FTP")
    {
        _facility = LOG_FTP;
    }
#endif
    else if(facilityString == "LOG_LOCAL0")
    {
        _facility = LOG_LOCAL0;
    }
    else if(facilityString == "LOG_LOCAL1")
    {
        _facility = LOG_LOCAL1;
    }
    else if(facilityString == "LOG_LOCAL2")
    {
        _facility = LOG_LOCAL2;
    }
    else if(facilityString == "LOG_LOCAL3")
    {
        _facility = LOG_LOCAL3;
    }
    else if(facilityString == "LOG_LOCAL4")
    {
        _facility = LOG_LOCAL4;
    }
    else if(facilityString == "LOG_LOCAL5")
    {
        _facility = LOG_LOCAL5;
    }
    else if(facilityString == "LOG_LOCAL6")
    {
        _facility = LOG_LOCAL6;
    }
    else if(facilityString == "LOG_LOCAL7")
    {
        _facility = LOG_LOCAL7;
    }
    else
    {
        throw InitializationException(__FILE__, __LINE__, "Invalid value for Ice.SyslogFacility: " + facilityString);
    }

    int logopt = LOG_PID | LOG_CONS;
    openlog(prefix.c_str(), logopt, _facility);
}

Ice::SysLoggerI::SysLoggerI(const string& prefix, int facility) :
    _facility(facility),
    _prefix(prefix)
{
    int logopt = LOG_PID | LOG_CONS;
    openlog(prefix.c_str(), logopt, facility);
}

Ice::SysLoggerI::~SysLoggerI()
{
    closelog();
}

void
Ice::SysLoggerI::print(const string& message)
{
    IceUtil::Mutex::Lock sync(*this);
    syslog(LOG_INFO, "%s", message.c_str());
}

void
Ice::SysLoggerI::trace(const string& category, const string& message)
{
    IceUtil::Mutex::Lock sync(*this);
    string s = category + ": " + message;
    syslog(LOG_INFO, "%s", s.c_str());
}

void
Ice::SysLoggerI::warning(const string& message)
{
    IceUtil::Mutex::Lock sync(*this);
    syslog(LOG_WARNING, "%s", message.c_str());
}

void
Ice::SysLoggerI::error(const string& message)
{
    IceUtil::Mutex::Lock sync(*this);
    syslog(LOG_ERR, "%s", message.c_str());
}

string
Ice::SysLoggerI::getPrefix()
{
    return _prefix;
}

Ice::LoggerPtr
Ice::SysLoggerI::cloneWithPrefix(const string& prefix)
{
    return new SysLoggerI(prefix, _facility);
}
