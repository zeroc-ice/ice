//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_LOCAL_EXCEPTION_H
#define ICE_LOCAL_EXCEPTION_H

#include "Exception.h"
#include "Ice/BuiltinSequences.h"
#include "Ice/Identity.h"
#include "Ice/Version.h"

#include <string>
#include <string_view>
#include <utility>

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace Ice
{
    /**
     * Abstract base class for all Ice run-time exceptions.
     * \headerfile Ice/Ice.h
     */
    class ICE_API LocalException : public Exception
    {
    public:
        using Exception::Exception;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;
    };

    /**
     * This exception is raised when a failure occurs during initialization.
     * \headerfile Ice/Ice.h
     */
    class ICE_API InitializationException : public LocalException
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
        InitializationException(const char* file, int line, std::string reason) noexcept
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

    /**
     * This exception indicates that a failure occurred while initializing a plug-in.
     * \headerfile Ice/Ice.h
     */
    class ICE_API PluginInitializationException : public LocalException
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
        PluginInitializationException(const char* file, int line, std::string reason) noexcept
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

    /**
     * An attempt was made to register something more than once with the Ice run time. This exception is raised if an
     * attempt is made to register a servant, servant locator, facet, value factory, plug-in, object adapter, object, or
     * user exception factory more than once for the same ID.
     * \headerfile Ice/Ice.h
     */
    class ICE_API AlreadyRegisteredException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param kindOfObject The kind of object that could not be removed: "servant", "facet", "object", "default
         * servant", "servant locator", "value factory", "plugin", "object adapter", "object adapter with router",
         * "replica group".
         * @param id The ID (or name) of the object that is registered already.
         */
        AlreadyRegisteredException(const char* file, int line, std::string kindOfObject, std::string id) noexcept
            : LocalException(file, line),
              kindOfObject(std::move(kindOfObject)),
              id(std::move(id))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&, const std::string&> ice_tuple() const noexcept
        {
            return std::tie(kindOfObject, id);
        }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The kind of object that could not be removed: "servant", "facet", "object", "default servant",
         * "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica
         * group".
         */
        std::string kindOfObject;
        /**
         * The ID (or name) of the object that is registered already.
         */
        std::string id;
    };

    /**
     * An attempt was made to find or deregister something that is not registered with the Ice run time or Ice locator.
     * This exception is raised if an attempt is made to remove a servant, servant locator, facet, value factory,
     * plug-in, object adapter, object, or user exception factory that is not currently registered. It's also raised if
     * the Ice locator can't find an object or object adapter when resolving an indirect proxy or when an object adapter
     * is activated. \headerfile Ice/Ice.h
     */
    class ICE_API NotRegisteredException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param kindOfObject The kind of object that could not be removed: "servant", "facet", "object", "default
         * servant", "servant locator", "value factory", "plugin", "object adapter", "object adapter with router",
         * "replica group".
         * @param id The ID (or name) of the object that could not be removed.
         */
        NotRegisteredException(const char* file, int line, std::string kindOfObject, std::string id) noexcept
            : LocalException(file, line),
              kindOfObject(std::move(kindOfObject)),
              id(std::move(id))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&, const std::string&> ice_tuple() const noexcept
        {
            return std::tie(kindOfObject, id);
        }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The kind of object that could not be removed: "servant", "facet", "object", "default servant",
         * "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica
         * group".
         */
        std::string kindOfObject;
        /**
         * The ID (or name) of the object that could not be removed.
         */
        std::string id;
    };

    /**
     * The operation can only be invoked with a twoway request. This exception is raised if an attempt is made to invoke
     * an operation with <code>ice_oneway</code>, <code>ice_batchOneway</code>, <code>ice_datagram</code>, or
     * <code>ice_batchDatagram</code> and the operation has a return value, out-parameters, or an exception
     * specification. \headerfile Ice/Ice.h
     */
    class ICE_API TwowayOnlyException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param operation The name of the operation that was invoked.
         */
        TwowayOnlyException(const char* file, int line, std::string operation) noexcept
            : LocalException(file, line),
              operation(std::move(operation))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&> ice_tuple() const noexcept { return std::tie(operation); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The name of the operation that was invoked.
         */
        std::string operation;
    };

    /**
     * An attempt was made to clone a class that does not support cloning. This exception is raised if
     * <code>ice_clone</code> is called on a class that is derived from an abstract Slice class (that is, a class
     * containing operations), and the derived class does not provide an implementation of the <code>ice_clone</code>
     * operation (C++ only).
     * \headerfile Ice/Ice.h
     */
    class ICE_API CloneNotImplementedException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised if an operation call on a server raises an unknown exception. For example, for C++, this
     * exception is raised if the server throws a C++ exception that is not directly or indirectly derived from
     * <code>Ice::LocalException</code> or <code>Ice::UserException</code>.
     * \headerfile Ice/Ice.h
     */
    class ICE_API UnknownException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param unknown This field is set to the textual representation of the unknown exception if available.
         */
        UnknownException(const char* file, int line, std::string unknown) noexcept
            : LocalException(file, line),
              unknown(std::move(unknown))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&> ice_tuple() const noexcept { return std::tie(unknown); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * This field is set to the textual representation of the unknown exception if available.
         */
        std::string unknown;
    };

    /**
     * This exception is raised if an operation call on a server raises a  local exception. Because local exceptions are
     * not transmitted by the Ice protocol, the client receives all local exceptions raised by the server as
     * {@link UnknownLocalException}. The only exception to this rule are all exceptions derived from
     * {@link RequestFailedException}, which are transmitted by the Ice protocol even though they are declared
     * <code>local</code>.
     * \headerfile Ice/Ice.h
     */
    class ICE_API UnknownLocalException : public UnknownException
    {
    public:
        using UnknownException::UnknownException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * An operation raised an incorrect user exception. This exception is raised if an operation raises a user exception
     * that is not declared in the exception's <code>throws</code> clause. Such undeclared exceptions are not
     * transmitted from the server to the client by the Ice protocol, but instead the client just gets an {@link
     * UnknownUserException}. This is necessary in order to not violate the contract established by an operation's
     * signature: Only local exceptions and user exceptions declared in the <code>throws</code> clause can be raised.
     * \headerfile Ice/Ice.h
     */
    class ICE_API UnknownUserException : public UnknownException
    {
    public:
        using UnknownException::UnknownException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised if the Ice library version does not match the version in the Ice header files.
     * \headerfile Ice/Ice.h
     */
    class ICE_API VersionMismatchException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised if the {@link Communicator} has been destroyed.
     * @see Communicator#destroy
     * \headerfile Ice/Ice.h
     */
    class ICE_API CommunicatorDestroyedException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised if an attempt is made to use a deactivated {@link ObjectAdapter}.
     * @see ObjectAdapter#deactivate
     * @see Communicator#shutdown
     * \headerfile Ice/Ice.h
     */
    class ICE_API ObjectAdapterDeactivatedException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param name Name of the adapter.
         */
        ObjectAdapterDeactivatedException(const char* file, int line, std::string name) noexcept
            : LocalException(file, line),
              name(std::move(name))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&> ice_tuple() const noexcept { return std::tie(name); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * Name of the adapter.
         */
        std::string name;
    };

    /**
     * This exception is raised if an {@link ObjectAdapter} cannot be activated. This happens if the {@link Locator}
     * detects another active {@link ObjectAdapter} with the same adapter id.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ObjectAdapterIdInUseException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param id Adapter ID.
         */
        ObjectAdapterIdInUseException(const char* file, int line, std::string id) noexcept
            : LocalException(file, line),
              id(std::move(id))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&> ice_tuple() const noexcept { return std::tie(id); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * Adapter ID.
         */
        std::string id;
    };

    /**
     * This exception is raised if no suitable endpoint is available.
     * \headerfile Ice/Ice.h
     */
    class ICE_API NoEndpointException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param proxy The stringified proxy for which no suitable endpoint is available.
         */
        NoEndpointException(const char* file, int line, std::string proxy) noexcept
            : LocalException(file, line),
              proxy(std::move(proxy))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&> ice_tuple() const noexcept { return std::tie(proxy); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The stringified proxy for which no suitable endpoint is available.
         */
        std::string proxy;
    };

    /**
     * This exception is raised if there was an error while parsing an endpoint.
     * \headerfile Ice/Ice.h
     */
    class ICE_API EndpointParseException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param str Describes the failure and includes the string that could not be parsed.
         */
        EndpointParseException(const char* file, int line, std::string str) noexcept
            : LocalException(file, line),
              str(std::move(str))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&> ice_tuple() const noexcept { return std::tie(str); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * Describes the failure and includes the string that could not be parsed.
         */
        std::string str;
    };

    /**
     * This exception is raised if there was an error while parsing an endpoint selection type.
     * \headerfile Ice/Ice.h
     */
    class ICE_API EndpointSelectionTypeParseException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param str Describes the failure and includes the string that could not be parsed.
         */
        EndpointSelectionTypeParseException(const char* file, int line, std::string str) noexcept
            : LocalException(file, line),
              str(std::move(str))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&> ice_tuple() const noexcept { return std::tie(str); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * Describes the failure and includes the string that could not be parsed.
         */
        std::string str;
    };

    /**
     * This exception is raised if there was an error while parsing a version.
     * \headerfile Ice/Ice.h
     */
    class ICE_API VersionParseException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param str Describes the failure and includes the string that could not be parsed.
         */
        VersionParseException(const char* file, int line, std::string str) noexcept
            : LocalException(file, line),
              str(std::move(str))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&> ice_tuple() const noexcept { return std::tie(str); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * Describes the failure and includes the string that could not be parsed.
         */
        std::string str;
    };

    /**
     * This exception is raised if there was an error while parsing a stringified identity.
     * \headerfile Ice/Ice.h
     */
    class ICE_API IdentityParseException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param str Describes the failure and includes the string that could not be parsed.
         */
        IdentityParseException(const char* file, int line, std::string str) noexcept
            : LocalException(file, line),
              str(std::move(str))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&> ice_tuple() const noexcept { return std::tie(str); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * Describes the failure and includes the string that could not be parsed.
         */
        std::string str;
    };

    /**
     * This exception is raised if there was an error while parsing a stringified proxy.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ProxyParseException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param str Describes the failure and includes the string that could not be parsed.
         */
        ProxyParseException(const char* file, int line, std::string str) noexcept
            : LocalException(file, line),
              str(std::move(str))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&> ice_tuple() const noexcept { return std::tie(str); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * Describes the failure and includes the string that could not be parsed.
         */
        std::string str;
    };

    /**
     * This exception is raised if an illegal identity is encountered.
     * \headerfile Ice/Ice.h
     */
    class ICE_API IllegalIdentityException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised to reject an illegal servant (typically a null servant).
     * \headerfile Ice/Ice.h
     */
    class ICE_API IllegalServantException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param reason Describes why this servant is illegal.
         */
        IllegalServantException(const char* file, int line, std::string reason) noexcept
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
         * Describes why this servant is illegal.
         */
        std::string reason;
    };

    /**
     * This exception is raised if a request failed. This exception, and all exceptions derived from
     * {@link RequestFailedException}, are transmitted by the Ice protocol, even though they are declared
     * <code>local</code>.
     * \headerfile Ice/Ice.h
     */
    class ICE_API RequestFailedException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param id The identity of the Ice Object to which the request was sent.
         * @param facet The facet to which the request was sent.
         * @param operation The operation name of the request.
         */
        RequestFailedException(
            const char* file,
            int line,
            Identity id,
            std::string facet,
            std::string operation) noexcept
            : LocalException(file, line),
              id(std::move(id)),
              facet(std::move(facet)),
              operation(std::move(operation))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const Identity&, const std::string&, const std::string&> ice_tuple() const noexcept
        {
            return std::tie(id, facet, operation);
        }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The identity of the Ice Object to which the request was sent.
         */
        Identity id;
        /**
         * The facet to which the request was sent.
         */
        std::string facet;
        /**
         * The operation name of the request.
         */
        std::string operation;
    };

    /**
     * This exception is raised if an object does not exist on the server, that is, if no facets with the given identity
     * exist.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ObjectNotExistException : public RequestFailedException
    {
    public:
        using RequestFailedException::RequestFailedException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised if no facet with the given name exists, but at least one facet with the given identity
     * exists.
     * \headerfile Ice/Ice.h
     */
    class ICE_API FacetNotExistException : public RequestFailedException
    {
    public:
        using RequestFailedException::RequestFailedException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised if an operation for a given object does not exist on the server. Typically this is
     * caused by either the client or the server using an outdated Slice specification. \headerfile Ice/Ice.h
     */
    class ICE_API OperationNotExistException : public RequestFailedException
    {
    public:
        using RequestFailedException::RequestFailedException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised if a system error occurred in the server or client process. There are many possible
     * causes for such a system exception. For details on the cause, {@link SyscallException#error} should be inspected.
     * \headerfile Ice/Ice.h
     */
    class ICE_API SyscallException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * Construct a SyscallException.
         * The file and line number are required for all local exceptions.
         * The error code is filled automatically with the value of the current error code.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         */
        SyscallException(const char* file, int line) noexcept;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param error The error number describing the system exception.
         */
        SyscallException(const char* file, int line, int error) noexcept : LocalException(file, line), error(error) {}

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const int&> ice_tuple() const noexcept { return std::tie(error); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The error number describing the system exception. For C++ and Unix, this is equivalent to <code>errno</code>.
         * For C++ and Windows, this is the value returned by <code>GetLastError()</code> or
         * <code>WSAGetLastError()</code>.
         */
        int error = 0;
    };

    /**
     * This exception indicates socket errors.
     * \headerfile Ice/Ice.h
     */
    class ICE_API SocketException : public SyscallException
    {
    public:
        using SyscallException::SyscallException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception indicates CFNetwork errors.
     * \headerfile Ice/Ice.h
     */
    class ICE_API CFNetworkException : public SocketException
    {
    public:
        using SocketException::SocketException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param error The error number describing the system exception.
         * @param domain The domain of the error.
         */
        CFNetworkException(const char* file, int line, int error, std::string domain) noexcept
            : SocketException(file, line, error),
              domain(std::move(domain))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const int&, const std::string&> ice_tuple() const noexcept { return std::tie(error, domain); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The domain of the error.
         */
        std::string domain;
    };

    /**
     * This exception indicates file errors.
     * \headerfile Ice/Ice.h
     */
    class ICE_API FileException : public SyscallException
    {
    public:
        using SyscallException::SyscallException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param path The path of the file responsible for the error.
         */
        FileException(const char* file, int line, std::string path) noexcept
            : SyscallException(file, line),
              path(std::move(path))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const int&, const std::string&> ice_tuple() const noexcept { return std::tie(error, path); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The path of the file responsible for the error.
         */
        std::string path;
    };

    /**
     * This exception indicates connection failures.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ConnectFailedException : public SocketException
    {
    public:
        using SocketException::SocketException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception indicates a connection failure for which the server host actively refuses a connection.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ConnectionRefusedException : public ConnectFailedException
    {
    public:
        using ConnectFailedException::ConnectFailedException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception indicates a lost connection.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ConnectionLostException : public SocketException
    {
    public:
        using SocketException::SocketException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception indicates a DNS problem. For details on the cause, {@link DNSException#error} should be inspected.
     * \headerfile Ice/Ice.h
     */
    class ICE_API DNSException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param error The error number describing the DNS problem.
         * @param host The host name that could not be resolved.
         */
        DNSException(const char* file, int line, int error, std::string host) noexcept
            : LocalException(file, line),
              error(error),
              host(std::move(host))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const int&, const std::string&> ice_tuple() const noexcept { return std::tie(error, host); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The error number describing the DNS problem. For C++ and Unix, this is equivalent to <code>h_errno</code>.
         * For C++ and Windows, this is the value returned by <code>WSAGetLastError()</code>.
         */
        int error = 0;
        /**
         * The host name that could not be resolved.
         */
        std::string host;
    };

    /**
     * This exception indicates that a connection was closed gracefully.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ConnectionClosedException : public LocalException
    {
    public:
        ConnectionClosedException(const char* file, int line, std::string message) noexcept
            : LocalException(file, line),
              message(std::move(message))
        {
        }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The error message.
         */
        std::string message;
    };

    /**
     * This exception indicates that a connection was aborted by the idle check.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ConnectionIdleException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception indicates a timeout condition.
     * \headerfile Ice/Ice.h
     */
    class ICE_API TimeoutException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception indicates a connection establishment timeout condition.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ConnectTimeoutException : public TimeoutException
    {
    public:
        using TimeoutException::TimeoutException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception indicates a connection closure timeout condition.
     * \headerfile Ice/Ice.h
     */
    class ICE_API CloseTimeoutException : public TimeoutException
    {
    public:
        using TimeoutException::TimeoutException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception indicates that an invocation failed because it timed out.
     * \headerfile Ice/Ice.h
     */
    class ICE_API InvocationTimeoutException : public TimeoutException
    {
    public:
        using TimeoutException::TimeoutException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception indicates that an asynchronous invocation failed because it was canceled explicitly by the user.
     * \headerfile Ice/Ice.h
     */
    class ICE_API InvocationCanceledException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * A generic exception base for all kinds of protocol error conditions.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ProtocolException : public LocalException
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
        ProtocolException(const char* file, int line, std::string reason) noexcept
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

    /**
     * This exception indicates that a message did not start with the expected magic number ('I', 'c', 'e', 'P').
     * \headerfile Ice/Ice.h
     */
    class ICE_API BadMagicException : public ProtocolException
    {
    public:
        using ProtocolException::ProtocolException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param reason The reason for the failure.
         * @param badMagic A sequence containing the first four bytes of the incorrect message.
         */
        BadMagicException(const char* file, int line, std::string reason, ByteSeq badMagic) noexcept
            : ProtocolException(file, line, std::move(reason)),
              badMagic(std::move(badMagic))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&, const ByteSeq&> ice_tuple() const noexcept { return std::tie(reason, badMagic); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * A sequence containing the first four bytes of the incorrect message.
         */
        ByteSeq badMagic;
    };

    /**
     * This exception indicates an unsupported protocol version.
     * \headerfile Ice/Ice.h
     */
    class ICE_API UnsupportedProtocolException : public ProtocolException
    {
    public:
        using ProtocolException::ProtocolException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param reason The reason for the failure.
         * @param bad The version of the unsupported protocol.
         * @param supported The version of the protocol that is supported.
         */
        UnsupportedProtocolException(
            const char* file,
            int line,
            std::string reason,
            ProtocolVersion bad,
            ProtocolVersion supported) noexcept
            : ProtocolException(file, line, std::move(reason)),
              bad(std::move(bad)),
              supported(std::move(supported))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&, const ProtocolVersion&, const ProtocolVersion&> ice_tuple() const noexcept
        {
            return std::tie(reason, bad, supported);
        }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The version of the unsupported protocol.
         */
        ProtocolVersion bad;
        /**
         * The version of the protocol that is supported.
         */
        ProtocolVersion supported;
    };

    /**
     * This exception indicates an unsupported data encoding version.
     * \headerfile Ice/Ice.h
     */
    class ICE_API UnsupportedEncodingException : public ProtocolException
    {
    public:
        using ProtocolException::ProtocolException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param reason The reason for the failure.
         * @param bad The version of the unsupported encoding.
         * @param supported The version of the encoding that is supported.
         */
        UnsupportedEncodingException(
            const char* file,
            int line,
            std::string reason,
            EncodingVersion bad,
            EncodingVersion supported) noexcept
            : ProtocolException(file, line, std::move(reason)),
              bad(std::move(bad)),
              supported(std::move(supported))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&, const EncodingVersion&, const EncodingVersion&> ice_tuple() const noexcept
        {
            return std::tie(reason, bad, supported);
        }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The version of the unsupported encoding.
         */
        EncodingVersion bad;
        /**
         * The version of the encoding that is supported.
         */
        EncodingVersion supported;
    };

    /**
     * This exception indicates that an unknown protocol message has been received.
     * \headerfile Ice/Ice.h
     */
    class ICE_API UnknownMessageException : public ProtocolException
    {
    public:
        using ProtocolException::ProtocolException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised if a message is received over a connection that is not yet validated.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ConnectionNotValidatedException : public ProtocolException
    {
    public:
        using ProtocolException::ProtocolException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception indicates that an unknown reply status has been received.
     * \headerfile Ice/Ice.h
     */
    class ICE_API UnknownReplyStatusException : public ProtocolException
    {
    public:
        using ProtocolException::ProtocolException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception indicates that the connection has been gracefully shut down by the server. The operation call that
     * caused this exception has not been executed by the server. In most cases you will not get this exception, because
     * the client will automatically retry the operation call in case the server shut down the connection. However, if
     * upon retry the server shuts down the connection again, and the retry limit has been reached, then this exception
     * is propagated to the application code. \headerfile Ice/Ice.h
     */
    class ICE_API CloseConnectionException : public ProtocolException
    {
    public:
        using ProtocolException::ProtocolException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised by an operation call if the application closes the connection locally using
     * {@link Connection#close}.
     * @see Connection#close
     * \headerfile Ice/Ice.h
     */
    class ICE_API ConnectionManuallyClosedException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param graceful True if the connection was closed gracefully, false otherwise.
         */
        ConnectionManuallyClosedException(const char* file, int line, bool graceful) noexcept
            : LocalException(file, line),
              graceful(graceful)
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const bool&> ice_tuple() const noexcept { return std::tie(graceful); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * True if the connection was closed gracefully, false otherwise.
         */
        bool graceful;
    };

    /**
     * This exception indicates that a message size is less than the minimum required size.
     * \headerfile Ice/Ice.h
     */
    class ICE_API IllegalMessageSizeException : public ProtocolException
    {
    public:
        using ProtocolException::ProtocolException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception indicates a problem with compressing or uncompressing data.
     * \headerfile Ice/Ice.h
     */
    class ICE_API CompressionException : public ProtocolException
    {
    public:
        using ProtocolException::ProtocolException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * A datagram exceeds the configured size. This exception is raised if a datagram exceeds the configured send or
     * receive buffer size, or exceeds the maximum payload size of a UDP packet (65507 bytes).
     * \headerfile Ice/Ice.h
     */
    class ICE_API DatagramLimitException : public ProtocolException
    {
    public:
        using ProtocolException::ProtocolException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised for errors during marshaling or unmarshaling data.
     * \headerfile Ice/Ice.h
     */
    class ICE_API MarshalException : public ProtocolException
    {
    public:
        using ProtocolException::ProtocolException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised if inconsistent data is received while unmarshaling a proxy.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ProxyUnmarshalException : public MarshalException
    {
    public:
        using MarshalException::MarshalException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised if an out-of-bounds condition occurs during unmarshaling.
     * \headerfile Ice/Ice.h
     */
    class ICE_API UnmarshalOutOfBoundsException : public MarshalException
    {
    public:
        using MarshalException::MarshalException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised if no suitable value factory was found during unmarshaling of a Slice class instance.
     * @see ValueFactory
     * @see Communicator#getValueFactoryManager
     * @see ValueFactoryManager#add
     * @see ValueFactoryManager#find
     * \headerfile Ice/Ice.h
     */
    class ICE_API NoValueFactoryException : public MarshalException
    {
    public:
        using MarshalException::MarshalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param reason The reason for the failure.
         * @param type The Slice type ID of the class instance for which no factory could be found.
         */
        NoValueFactoryException(const char* file, int line, std::string reason, std::string type) noexcept
            : MarshalException(file, line, std::move(reason)),
              type(std::move(type))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&, const std::string&> ice_tuple() const noexcept { return std::tie(reason, type); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The Slice type ID of the class instance for which no factory could be found.
         */
        std::string type;
    };

    /**
     * This exception is raised if the type of an unmarshaled Slice class instance does not match its expected type.
     * This can happen if client and server are compiled with mismatched Slice definitions or if a class of the wrong
     * type is passed as a parameter or return value using dynamic invocation. This exception can also be raised if
     * IceStorm is used to send Slice class instances and an operation is subscribed to the wrong topic. \headerfile
     * Ice/Ice.h
     */
    class ICE_API UnexpectedObjectException : public MarshalException
    {
    public:
        using MarshalException::MarshalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param reason The reason for the failure.
         * @param type The Slice type ID of the class instance that was unmarshaled.
         * @param expectedType The Slice type ID that was expected by the receiving operation.
         */
        UnexpectedObjectException(
            const char* file,
            int line,
            std::string reason,
            std::string type,
            std::string expectedType) noexcept
            : MarshalException(file, line, std::move(reason)),
              type(std::move(type)),
              expectedType(std::move(expectedType))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&, const std::string&, const std::string&> ice_tuple() const noexcept
        {
            return std::tie(reason, type, expectedType);
        }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The Slice type ID of the class instance that was unmarshaled.
         */
        std::string type;
        /**
         * The Slice type ID that was expected by the receiving operation.
         */
        std::string expectedType;
    };

    /**
     * This exception is raised when Ice receives a request or reply message whose size exceeds the limit specified by
     * the <code>Ice.MessageSizeMax</code> property. \headerfile Ice/Ice.h
     */
    class ICE_API MemoryLimitException : public MarshalException
    {
    public:
        using MarshalException::MarshalException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised when a string conversion to or from UTF-8 fails during marshaling or unmarshaling.
     * \headerfile Ice/Ice.h
     */
    class ICE_API StringConversionException : public MarshalException
    {
    public:
        using MarshalException::MarshalException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception indicates a malformed data encapsulation.
     * \headerfile Ice/Ice.h
     */
    class ICE_API EncapsulationException : public MarshalException
    {
    public:
        using MarshalException::MarshalException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };

    /**
     * This exception is raised if an unsupported feature is used. The unsupported feature string contains the name of
     * the unsupported feature. \headerfile Ice/Ice.h
     */
    class ICE_API FeatureNotSupportedException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         * @param unsupportedFeature The name of the unsupported feature.
         */
        FeatureNotSupportedException(const char* file, int line, std::string unsupportedFeature) noexcept
            : LocalException(file, line),
              unsupportedFeature(std::move(unsupportedFeature))
        {
        }

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::string&> ice_tuple() const noexcept { return std::tie(unsupportedFeature); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The name of the unsupported feature.
         */
        std::string unsupportedFeature;
    };

    /**
     * This exception indicates a failure in a security subsystem, such as the IceSSL plug-in.
     * \headerfile Ice/Ice.h
     */
    class ICE_API SecurityException : public LocalException
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
        SecurityException(const char* file, int line, std::string reason) noexcept
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

    /**
     * This exception indicates that an attempt has been made to change the connection properties of a fixed proxy.
     * \headerfile Ice/Ice.h
     */
    class ICE_API FixedProxyException : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_print(std::ostream& stream) const override;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
