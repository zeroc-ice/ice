// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
