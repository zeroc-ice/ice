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
    // A logger that works in tandem with a "Logger" admin facet.
    class LoggerAdminLogger : public Ice::Logger
    {
    public:
        ~LoggerAdminLogger() override;

        // Returns the associated Admin facet
        [[nodiscard]] virtual Ice::ObjectPtr getFacet() const = 0;

        // Once detach is called, the logger only logs to the local logger.
        virtual void detach() = 0;
    };
    using LoggerAdminLoggerPtr = std::shared_ptr<LoggerAdminLogger>;

    LoggerAdminLoggerPtr createLoggerAdminLogger(const Ice::PropertiesPtr&, const Ice::LoggerPtr&);
}

#endif
