// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SysLoggerI.h>
#include <syslog.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
Ice::SysLoggerI::trace(const string& category, const string& message)
{
    JTCSyncT<JTCMutex> sync(*this);
    string s = "[ " + category + ": " + message + " ]";
    string::size_type idx = 0;
    while ((idx = s.find("\n", idx)) != string::npos)
    {
	s.insert(idx + 1, "  ");
	++idx;
    }
    syslog(LOG_INFO, "%s", s.c_str());
}

void
Ice::SysLoggerI::warning(const string& message)
{
    JTCSyncT<JTCMutex> sync(*this);
    syslog(LOG_WARNING, "%s", message.c_str());
}

void
Ice::SysLoggerI::error(const string& message)
{
    JTCSyncT<JTCMutex> sync(*this);
    syslog(LOG_ERR, "%s", message.c_str());
}
