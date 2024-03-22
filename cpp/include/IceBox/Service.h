//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __IceBox_Service_h__
#define __IceBox_Service_h__

#include <IceUtil/PushDisableWarnings.h>
#include "Ice/Ice.h"
#include "Config.h"

#ifndef ICEBOX_API
#    if defined(ICE_STATIC_LIBS)
#        define ICEBOX_API /**/
#    elif defined(ICEBOX_API_EXPORTS)
#        define ICEBOX_API ICE_DECLSPEC_EXPORT
#    else
#        define ICEBOX_API ICE_DECLSPEC_IMPORT
#    endif
#endif
namespace IceBox
{
    class Service;
}

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
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param reason The reason for the failure.
         */
        FailureException(const char* file, int line, std::string reason) noexcept
            : LocalException(file, line),
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
}

namespace IceBox
{
    /**
     * An application service managed by a {@link ServiceManager}.
     * \headerfile IceBox/IceBox.h
     */
    class ICE_CLASS(ICEBOX_API) Service
    {
    public:
        ICE_MEMBER(ICEBOX_API) virtual ~Service();

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
}

/// \cond INTERNAL
namespace IceBox
{
    using ServicePtr = std::shared_ptr<Service>;
}
/// \endcond

#include <IceUtil/PopDisableWarnings.h>
#endif
