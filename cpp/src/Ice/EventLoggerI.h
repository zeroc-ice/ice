// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef ICE_EVENT_LOGGER_I_H
#define ICE_EVENT_LOGGER_I_H

#include <Ice/Logger.h>

namespace Ice
{

class EventLoggerI : public Logger
{
public:

    EventLoggerI(const std::string&);
    ~EventLoggerI();

    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);

    static void setModuleHandle(HMODULE);

private:

    std::string _appName;
    HANDLE _source;
    static HMODULE _module;
};

}

#endif
