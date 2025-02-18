// Copyright (c) ZeroC, Inc.

#ifndef ICE_LOGGER_MIDDLEWARE_H
#define ICE_LOGGER_MIDDLEWARE_H

#include "Ice/Initialize.h" // for ToStringMode
#include "Ice/Logger.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Object.h"

namespace IceInternal
{
    /// A middleware that logs warnings for failed dispatches.
    class LoggerMiddleware final : public Ice::Object, public std::enable_shared_from_this<LoggerMiddleware>
    {
    public:
        LoggerMiddleware(
            Ice::ObjectPtr next,
            Ice::LoggerPtr logger,
            int traceLevel,
            const char* traceCat,
            int warningLevel,
            Ice::ToStringMode toStringMode);

        void dispatch(Ice::IncomingRequest&, std::function<void(Ice::OutgoingResponse)>) final;

    private:
        void logDispatch(Ice::ReplyStatus replyStatus, const Ice::Current& current) const noexcept;
        void logDispatchFailed(std::string_view message, const Ice::Current& current) const noexcept;
        void logDispatchFailed(const Ice::LocalException& localException, const Ice::Current& current) const noexcept;
        void printTarget(Ice::LoggerOutputBase& out, const Ice::Current& current) const noexcept;

        Ice::ObjectPtr _next;
        Ice::LoggerPtr _logger;
        int _traceLevel;
        const char* _traceCat;
        int _warningLevel;
        Ice::ToStringMode _toStringMode;
    };
}

#endif
