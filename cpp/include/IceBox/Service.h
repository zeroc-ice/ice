//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEBOX_SERVICE_H
#define ICEBOX_SERVICE_H

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/LocalObject.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <Ice/Proxy.h>
#include <Ice/Object.h>
#include <Ice/GCObject.h>
#include <Ice/Value.h>
#include <Ice/Incoming.h>
#include <Ice/FactoryTableInit.h>
#include <IceUtil/ScopedArray.h>
#include <Ice/Optional.h>
#include <Ice/ExceptionHelpers.h>
#include <Ice/BuiltinSequences.h>
#include <Ice/CommunicatorF.h>
#include <Ice/PropertiesF.h>
#include <IceUtil/UndefSysMacros.h>
#include <IceBox/Config.h>

#ifndef ICEBOX_API
#   if defined(ICE_STATIC_LIBS)
#       define ICEBOX_API /**/
#   elif defined(ICEBOX_API_EXPORTS)
#       define ICEBOX_API ICE_DECLSPEC_EXPORT
#   else
#       define ICEBOX_API ICE_DECLSPEC_IMPORT
#   endif
#endif

#ifdef ICE_CPP11_MAPPING // C++11 mapping

namespace IceBox
{

class Service;

}

namespace IceBox
{

/**
 * This exception is a general failure notification. It is thrown
 * for errors such as a service encountering an error during
 * initialization, or the service manager being unable
 * to load a service executable.
 * \headerfile IceBox/IceBox.h
 */
class ICE_CLASS(ICEBOX_API) FailureException : public ::Ice::LocalExceptionHelper<FailureException, ::Ice::LocalException>
{
public:

    ICE_MEMBER(ICEBOX_API) virtual ~FailureException();

    FailureException(const FailureException&) = default;

    /**
     * The file and line number are required for all local exceptions.
     * @param file The file name in which the exception was raised, typically __FILE__.
     * @param line The line number at which the exception was raised, typically __LINE__.
     */
    FailureException(const char* file, int line) : ::Ice::LocalExceptionHelper<FailureException, ::Ice::LocalException>(file, line)
    {
    }

    /**
     * One-shot constructor to initialize all data members.
     * The file and line number are required for all local exceptions.
     * @param file The file name in which the exception was raised, typically __FILE__.
     * @param line The line number at which the exception was raised, typically __LINE__.
     * @param reason The reason for the failure.
     */
    FailureException(const char* file, int line, const ::std::string& reason) : ::Ice::LocalExceptionHelper<FailureException, ::Ice::LocalException>(file, line),
        reason(::std::move(reason))
    {
    }

    /**
     * Obtains a tuple containing all of the exception's data members.
     * @return The data members in a tuple.
     */
    std::tuple<const ::std::string&> ice_tuple() const
    {
        return std::tie(reason);
    }

    /**
     * Obtains the Slice type ID of this exception.
     * @return The fully-scoped type ID.
     */
    ICE_MEMBER(ICEBOX_API) static const ::std::string& ice_staticId();
    /**
     * Prints this exception to the given stream.
     * @param stream The target stream.
     */
    ICE_MEMBER(ICEBOX_API) virtual void ice_print(::std::ostream& stream) const override;

    /**
     * The reason for the failure.
     */
    ::std::string reason;
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
     * Start the service. The given communicator is created by the
     * {@link ServiceManager} for use by the service. This communicator may
     * also be used by other services, depending on the service
     * configuration.
     *
     * <p class="Note">The {@link ServiceManager} owns this communicator, and is
     * responsible for destroying it.
     * @param name The service's name, as determined by the
     * configuration.
     * @param communicator A communicator for use by the service.
     * @param args The service arguments that were not converted into
     * properties.
     * @throws IceBox::FailureException Raised if {@link #start} failed.
     */
    virtual void start(const ::std::string& name, const ::std::shared_ptr<::Ice::Communicator>& communicator, const ::Ice::StringSeq& args) = 0;

    /**
     * Stop the service.
     */
    virtual void stop() = 0;
};

}

/// \cond INTERNAL
namespace IceBox
{

using ServicePtr = ::std::shared_ptr<Service>;

}
/// \endcond

#else // C++98 mapping

namespace IceBox
{

class Service;
/// \cond INTERNAL
ICEBOX_API ::Ice::LocalObject* upCast(Service*);
/// \endcond
typedef ::IceInternal::Handle< Service> ServicePtr;

}

namespace IceBox
{

/**
 * This exception is a general failure notification. It is thrown
 * for errors such as a service encountering an error during
 * initialization, or the service manager being unable
 * to load a service executable.
 * \headerfile IceBox/IceBox.h
 */
class ICEBOX_API FailureException : public ::Ice::LocalException
{
public:

    /**
     * The file and line number are required for all local exceptions.
     * @param file The file name in which the exception was raised, typically __FILE__.
     * @param line The line number at which the exception was raised, typically __LINE__.
     */
    FailureException(const char* file, int line);
    /**
     * One-shot constructor to initialize all data members.
     * The file and line number are required for all local exceptions.
     * @param file The file name in which the exception was raised, typically __FILE__.
     * @param line The line number at which the exception was raised, typically __LINE__.
     * @param reason The reason for the failure.
     */
    FailureException(const char* file, int line, const ::std::string& reason);
    virtual ~FailureException() throw();

    /**
     * Obtains the Slice type ID of this exception.
     * @return The fully-scoped type ID.
     */
    virtual ::std::string ice_id() const;
    /**
     * Prints this exception to the given stream.
     * @param stream The target stream.
     */
    virtual void ice_print(::std::ostream& stream) const;
    /**
     * Polymporphically clones this exception.
     * @return A shallow copy of this exception.
     */
    virtual FailureException* ice_clone() const;
    /**
     * Throws this exception.
     */
    virtual void ice_throw() const;

    /**
     * The reason for the failure.
     */
    ::std::string reason;
};

}

namespace IceBox
{

/**
 * An application service managed by a {@link ServiceManager}.
 * \headerfile IceBox/IceBox.h
 */
class ICEBOX_API Service : public virtual ::Ice::LocalObject
{
public:

    typedef ServicePtr PointerType;

    virtual ~Service();

    /**
     * Start the service. The given communicator is created by the
     * {@link ServiceManager} for use by the service. This communicator may
     * also be used by other services, depending on the service
     * configuration.
     *
     * <p class="Note">The {@link ServiceManager} owns this communicator, and is
     * responsible for destroying it.
     * @param name The service's name, as determined by the
     * configuration.
     * @param communicator A communicator for use by the service.
     * @param args The service arguments that were not converted into
     * properties.
     * @throws IceBox::FailureException Raised if {@link #start} failed.
     */
    virtual void start(const ::std::string& name, const ::Ice::CommunicatorPtr& communicator, const ::Ice::StringSeq& args) = 0;

    /**
     * Stop the service.
     */
    virtual void stop() = 0;
};

/// \cond INTERNAL
inline bool operator==(const Service& lhs, const Service& rhs)
{
    return static_cast<const ::Ice::LocalObject&>(lhs) == static_cast<const ::Ice::LocalObject&>(rhs);
}

inline bool operator<(const Service& lhs, const Service& rhs)
{
    return static_cast<const ::Ice::LocalObject&>(lhs) < static_cast<const ::Ice::LocalObject&>(rhs);
}
/// \endcond

}

#endif

#include <IceUtil/PopDisableWarnings.h>

#endif
