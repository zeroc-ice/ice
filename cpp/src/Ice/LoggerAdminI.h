// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

namespace Ice
{

//
// A logger that works in tandem with a "Logger" admin facet.
//
class ICE_API LoggerAdminLogger : public Ice::Logger
{
public:

    //
    // Add the associated LoggerAdmin facet to this communicator
    //
    virtual void addAdminFacet(const CommunicatorPtr&) = 0;


    //
    // Destroy this logger, in particular join any thread
    // that this logger may have started
    //
    virtual void destroy() = 0;
    
};
typedef IceInternal::Handle<LoggerAdminLogger> LoggerAdminLoggerPtr;

}

namespace IceInternal
{

Ice::LoggerAdminLoggerPtr 
createLoggerAdminLogger(const std::string&, const Ice::PropertiesPtr&, const Ice::LoggerPtr&);

}

#endif
