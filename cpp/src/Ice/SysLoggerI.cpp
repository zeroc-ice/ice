// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/SysLoggerI.h>
#include <syslog.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

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
