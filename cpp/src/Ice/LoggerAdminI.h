// Copyright (c) ZeroC, Inc.

#ifndef ICE_LOGGER_ADMIN_I_H
#define ICE_LOGGER_ADMIN_I_H

#include "Ice/CommunicatorF.h"
#include "Ice/Config.h"
#include "Ice/Logger.h"
#include "Ice/ObjectF.h"
#include "Ice/PropertiesF.h"

namespace IceInternal
{
    //
    // A logger that works in tandem with a "Logger" admin facet.
    //
    class LoggerAdminLogger : public Ice::Logger
    {
    public:
        ~LoggerAdminLogger() override;

        //
        // Return the associated Admin facet
        //
        [[nodiscard]] virtual Ice::ObjectPtr getFacet() const = 0;

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
