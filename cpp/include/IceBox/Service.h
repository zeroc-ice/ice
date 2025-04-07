// Copyright (c) ZeroC, Inc.

#ifndef ICEBOX_SERVICE_H
#define ICEBOX_SERVICE_H

#include "Ice/Ice.h"

#ifndef ICEBOX_API
#    if defined(ICEBOX_API_EXPORTS)
#        define ICEBOX_API ICE_DECLSPEC_EXPORT
#    else
#        define ICEBOX_API ICE_DECLSPEC_IMPORT
#    endif
#endif

namespace IceBox
{
    /// The exception that is thrown when an IceBox service fails to start.
    /// @remark You can throw any exception from your implementation of Service::start. This exception is provided for
    /// backward compatibility with earlier versions of IceBox.
    /// @headerfile IceBox/IceBox.h
    class ICEBOX_API FailureException final : public Ice::LocalException
    {
    public:
        using Ice::LocalException::LocalException;

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// Represents an IceBox service that you implement and that the IceBox service manager starts and later stops.
    /// The same service can be started and stopped multiple times.
    /// @headerfile IceBox/IceBox.h
    class ICEBOX_API Service
    {
    public:
        virtual ~Service();

        /// Starts the service.
        /// @param name The service's name, as specified in configuration.
        /// @param communicator A communicator for use by the service. The IceBox service manager creates this
        /// communicator when it starts, and destroys this communicator when it shuts down.
        /// @param args The service arguments that were not converted into properties of @p communicator.
        virtual void
        start(const std::string& name, const Ice::CommunicatorPtr& communicator, const Ice::StringSeq& args) = 0;

        /// Stops the service.
        virtual void stop() = 0;
    };

    /// A shared pointer to a Service.
    using ServicePtr = std::shared_ptr<Service>;
}

#endif
