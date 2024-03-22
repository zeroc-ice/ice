//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEBOX_SERVICE_H
#define ICEBOX_SERVICE_H

#include "Config.h"
#include "Ice/Ice.h"

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace IceBox
{
    /**
     * This exception is a general failure notification. It is thrown for errors such as a service encountering an error
     * during initialization, or the service manager being unable to load a service executable.
     * \headerfile IceBox/IceBox.h
     */
    class ICEBOX_API FailureException : public Ice::LocalException
    {
    public:
        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param reason The reason for the failure.
         */
        FailureException(const char* file, int line, std::string reason) noexcept
            : Ice::LocalException(file, line),
              reason(std::move(reason))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&> ice_tuple() const noexcept { return std::tie(reason); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The reason for the failure.
         */
        std::string reason;
    };

    /**
     * An application service managed by a {@link ServiceManager}.
     * \headerfile IceBox/IceBox.h
     */
    class Service
    {
    public:
        virtual ~Service() = default;

        /**
         * Start the service. The given communicator is created by the {@link ServiceManager} for use by the service.
         * This communicator may also be used by other services, depending on the service configuration. <p
         * class="Note">The {@link ServiceManager} owns this communicator, and is responsible for destroying it.
         * @param name The service's name, as determined by the configuration.
         * @param communicator A communicator for use by the service.
         * @param args The service arguments that were not converted into properties.
         * @throws IceBox::FailureException Raised if {@link #start} failed.
         */
        virtual void start(
            const std::string& name,
            const std::shared_ptr<Ice::Communicator>& communicator,
            const Ice::StringSeq& args) = 0;

        /**
         * Stop the service.
         */
        virtual void stop() = 0;
    };

    using ServicePtr = std::shared_ptr<Service>;
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
