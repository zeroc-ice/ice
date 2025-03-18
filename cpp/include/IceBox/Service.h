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
    /**
     * This exception is a general failure notification. It is thrown for errors such as a service encountering an error
     * during initialization, or the service manager being unable to load a service executable.
     * \headerfile IceBox/IceBox.h
     */
    class ICEBOX_API FailureException final : public Ice::LocalException
    {
    public:
        using Ice::LocalException::LocalException;

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /**
     * An application service managed by a {@link ServiceManager}.
     * \headerfile IceBox/IceBox.h
     */
    class ICEBOX_API Service
    {
    public:
        virtual ~Service();

        /**
         * Start the service. The given communicator is created by the {@link ServiceManager} for use by the service.
         * This communicator may also be used by other services, depending on the service configuration. <p
         * class="Note">The {@link ServiceManager} owns this communicator, and is responsible for destroying it.
         * @param name The service's name, as determined by the configuration.
         * @param communicator A communicator for use by the service.
         * @param args The service arguments that were not converted into properties.
         * @throws IceBox::FailureException Raised if {@link #start} failed.
         */
        virtual void
        start(const std::string& name, const Ice::CommunicatorPtr& communicator, const Ice::StringSeq& args) = 0;

        /**
         * Stop the service.
         */
        virtual void stop() = 0;
    };

    /// A shared pointer to a Service.
    using ServicePtr = std::shared_ptr<Service>;
}

#endif
