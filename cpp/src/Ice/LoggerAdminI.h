//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_LOGGER_ADMIN_I_H
#define ICE_LOGGER_ADMIN_I_H

#include <Ice/Config.h>
#include <Ice/Logger.h>
#include <Ice/PropertiesF.h>
#include <Ice/CommunicatorF.h>
#include "Ice/ObjectF.h"

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
        virtual std::shared_ptr<Ice::Object> getFacet() const = 0;

        //
        // Destroy this logger, in particular join any thread
        // that this logger may have started
        //
        virtual void destroy() = 0;
    };
    using LoggerAdminLoggerPtr = std::shared_ptr<LoggerAdminLogger>;

    LoggerAdminLoggerPtr createLoggerAdminLogger(const Ice::PropertiesPtr&, const Ice::LoggerPtr&);
}

#endif
