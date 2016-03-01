// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_LOGGER_ADMIN_I_H
#define ICE_LOGGER_ADMIN_I_H

#include <Ice/Config.h>
#include <Ice/Logger.h>
#include <Ice/PropertiesF.h>
#include <Ice/CommunicatorF.h>

namespace IceInternal
{

//
// A logger that works in tandem with a "Logger" admin facet.
//
class LoggerAdminLogger : public Ice::Logger
{
public:

    //
    // Return the associated Admin facet
    //
    virtual Ice::ObjectPtr getFacet() const = 0;

    //
    // Destroy this logger, in particular join any thread
    // that this logger may have started
    //
    virtual void destroy() = 0; 
};
typedef Handle<LoggerAdminLogger> LoggerAdminLoggerPtr;

LoggerAdminLoggerPtr 
createLoggerAdminLogger(const Ice::PropertiesPtr&, const Ice::LoggerPtr&);

}

#endif
