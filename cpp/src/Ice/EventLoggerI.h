// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    virtual void print(const std::string&);
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);

    static void setModuleHandle(HMODULE);

private:

    HANDLE _source;
    static HMODULE _module;
};

}

#endif
