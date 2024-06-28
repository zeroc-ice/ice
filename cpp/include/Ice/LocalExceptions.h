//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_LOCAL_EXCEPTIONS_H
#define ICE_LOCAL_EXCEPTIONS_H

#include "Ice/BuiltinSequences.h"
#include "Ice/Identity.h"
#include "Ice/Version.h"
#include "LocalException.h"

#include <memory>
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

// This file contains most exceptions derived from Ice::LocalException.

namespace Ice
{
    /**
     * This exception is raised if a request failed. This exception, and all exceptions derived from
     * {@link RequestFailedException}, are transmitted by the Ice protocol, even though they are declared
     * <code>local</code>.
     * \headerfile Ice/Ice.h
     */
    class ICE_API RequestFailedException : public LocalException
    {
    public:
        /**
         * The identity of the Ice Object to which the request was sent.
         */
        const Identity& id() const noexcept { return *_id; }

        /**
         * The facet to which the request was sent.
         */
        const std::string& facet() const noexcept { return *_facet; }

        /**
         * The operation name of the request.
         */
        const std::string& operation() const noexcept { return *_operation; }

    protected:
        /**
         * Constructs a RequestFailedException with a custom error message.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         * @param message The error message adopted by this exception and returned by what().
         * @param id The identity of the Ice Object to which the request was sent.
         * @param facet The facet to which the request was sent.
         * @param operation The operation name of the request.
         */
        RequestFailedException(
            const char* file,
            int line,
            std::string message,
            Identity id,
            std::string facet,
            std::string operation)
            : LocalException(file, line, std::move(message)),
              _id(std::make_shared<Identity>(std::move(id))),
              _facet(std::make_shared<std::string>(std::move(facet))),
              _operation(std::make_shared<std::string>(std::move(operation)))
        {
        }

        /**
         * Constructs a RequestFailedException without specifying the details of the current request. The details will
         * be filled-in automatically by the Ice runtime before marshaling the exception.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         * @param message The error message adopted by this exception and returned by what().
         */
        RequestFailedException(const char* file, int line, std::string message)
            : RequestFailedException(file, line, std::move(message), Identity{}, "", "")
        {
        }

    private:
        std::shared_ptr<Identity> _id;
        std::shared_ptr<std::string> _facet;
        std::shared_ptr<std::string> _operation;
    };

    /**
     * This exception is raised if an object does not exist on the server, that is, if no facets with the given identity
     * exist.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ObjectNotExistException final : public RequestFailedException
    {
    public:
        /**
         * Constructs an ObjectNotExistException.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         * @param id The identity of the Ice Object to which the request was sent.
         * @param facet The facet to which the request was sent.
         * @param operation The operation name of the request.
         */
        ObjectNotExistException(const char* file, int line, Identity id, std::string facet, std::string operation);

        /**
         * Constructs an ObjectNotExistException without specifying the details of the current request. The details will
         * be filled-in automatically by the Ice runtime before marshaling the exception.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         */
        ObjectNotExistException(const char* file, int line);

        const char* ice_id() const noexcept final;
    };

    /**
     * This exception is raised if no facet with the given name exists, but at least one facet with the given identity
     * exists.
     * \headerfile Ice/Ice.h
     */
    class ICE_API FacetNotExistException final : public RequestFailedException
    {
    public:
        /**
         * Constructs a FacetNotExistException.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         * @param id The identity of the Ice Object to which the request was sent.
         * @param facet The facet to which the request was sent.
         * @param operation The operation name of the request.
         */
        FacetNotExistException(const char* file, int line, Identity id, std::string facet, std::string operation);

        /**
         * Constructs a FacetNotExistException without specifying the details of the current request. The details will
         * be filled-in automatically by the Ice runtime before marshaling the exception.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         */
        FacetNotExistException(const char* file, int line);

        const char* ice_id() const noexcept final;
    };

    /**
     * This exception is raised if an operation for a given object does not exist on the server. Typically this is
     * caused by either the client or the server using outdated Slice definitions.
     * \headerfile Ice/Ice.h
     */
    class ICE_API OperationNotExistException final : public RequestFailedException
    {
    public:
        /**
         * Constructs an OperationNotExistException.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         * @param id The identity of the Ice Object to which the request was sent.
         * @param facet The facet to which the request was sent.
         * @param operation The operation name of the request.
         */
        OperationNotExistException(const char* file, int line, Identity id, std::string facet, std::string operation);

        /**
         * Constructs an OperationNotExistException without specifying the details of the current request. The details
         * will be filled-in automatically by the Ice runtime before marshaling the exception.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         */
        OperationNotExistException(const char* file, int line);

        const char* ice_id() const noexcept final;
    };

    /**
     * This exception is raised when a dispatch fails with an unknown exception. For example, for C++, this
     * exception is raised if the dispatch throws a C++ exception that is not directly or indirectly derived from
     * <code>Ice::LocalException</code> or <code>Ice::UserException</code>.
     * \headerfile Ice/Ice.h
     */
    class ICE_API UnknownException : public LocalException
    {
    public:
        /**
         * Constructs an UnknownException.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         * @param message The message returned by what().
         */
        UnknownException(const char* file, int line, std::string message)
            : LocalException(file, line, std::move(message))
        {
        }

        const char* ice_id() const noexcept override;
    };

    /**
     * This exception is raised if an operation call on a server raises a local exception. Because local exceptions are
     * not transmitted by the Ice protocol, the client receives all local exceptions raised by the server as
     * {@link UnknownLocalException}. The only exception to this rule are all exceptions derived from
     * {@link RequestFailedException}, which are transmitted by the Ice protocol even though they are declared
     * <code>local</code>.
     * \headerfile Ice/Ice.h
     */
    class ICE_API UnknownLocalException final : public UnknownException
    {
    public:
        using UnknownException::UnknownException;

        const char* ice_id() const noexcept final;
    };

    /**
     * An operation raised an incorrect user exception. This exception is raised if an operation raises a user exception
     * that is not declared in the exception's <code>throws</code> clause. Such undeclared exceptions are not
     * transmitted from the server to the client by the Ice protocol, but instead the client just gets an {@link
     * UnknownUserException}. This is necessary in order to not violate the contract established by an operation's
     * signature: Only local exceptions and user exceptions declared in the <code>throws</code> clause can be raised.
     * \headerfile Ice/Ice.h
     */
    class ICE_API UnknownUserException final : public UnknownException
    {
    public:
        using UnknownException::UnknownException;

        const char* ice_id() const noexcept final;
    };

    /**
     * Reports a failure that occurred while parsing a string.
     *  \headerfile Ice/Ice.h
     */
    class ICE_API ParseException final : public LocalException
    {
    public:
        /**
         * Constructs a ParseException.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         * @param message The message returned by what().
         */
        ParseException(const char* file, int line, std::string message) : LocalException(file, line, std::move(message))
        {
        }

        const char* ice_id() const noexcept final;
    };

    /**
     * A generic exception base for all kinds of protocol error conditions.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ProtocolException : public LocalException
    {
    public:
        /**
         * Constructs a ProtocolException.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         * @param message The message returned by what().
         */
        ProtocolException(const char* file, int line, std::string message)
            : LocalException(file, line, std::move(message))
        {
        }

        const char* ice_id() const noexcept override;
    };

    /**
     * This exception indicates that the connection has been gracefully shut down by the server. The operation call that
     * caused this exception has not been executed by the server. In most cases you will not get this exception, because
     * the client will automatically retry the operation call in case the server shut down the connection. However, if
     * upon retry the server shuts down the connection again, and the retry limit has been reached, then this exception
     * is propagated to the application code.
     * \headerfile Ice/Ice.h
     */
    class ICE_API CloseConnectionException final : public ProtocolException
    {
    public:
        CloseConnectionException(const char* file, int line)
            : ProtocolException(file, line, "connection closed by the peer")
        {
        }

        const char* ice_id() const noexcept final;
    };

    /**
     * A datagram exceeds the configured size. This exception is raised if a datagram exceeds the configured send or
     * receive buffer size, or exceeds the maximum payload size of a UDP packet (65507 bytes).
     * \headerfile Ice/Ice.h
     */
    class ICE_API DatagramLimitException final : public ProtocolException
    {
    public:
        DatagramLimitException(const char* file, int line)
            : ProtocolException(file, line, "datagram size exceeds configured limit")
        {
        }

        const char* ice_id() const noexcept final;
    };

    /**
     * This exception is raised for errors during marshaling or unmarshaling data.
     * \headerfile Ice/Ice.h
     */
    class ICE_API MarshalException : public ProtocolException
    {
    public:
        using ProtocolException::ProtocolException;

        const char* ice_id() const noexcept final;
    };

    /**
     * This exception is raised if an unsupported feature is used.
     * \headerfile Ice/Ice.h
     */
    class ICE_API FeatureNotSupportedException final : public LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * Constructs a FeatureNotSupportedException.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         * @param message The message returned by what().
         */
        FeatureNotSupportedException(const char* file, int line, std::string message)
            : LocalException(file, line, std::move(message))
        {
        }

        const char* ice_id() const noexcept final;
    };

    //
    // Below: not refactored yet
    //

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
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
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

        const char* ice_id() const noexcept override;

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
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
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

        const char* ice_id() const noexcept override;

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
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
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

        const char* ice_id() const noexcept override;

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
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
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

        const char* ice_id() const noexcept override;

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
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
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

        const char* ice_id() const noexcept override;

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

        const char* ice_id() const noexcept override;

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

        const char* ice_id() const noexcept override;

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

        const char* ice_id() const noexcept override;

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
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
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

        const char* ice_id() const noexcept override;

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
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
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

        const char* ice_id() const noexcept override;

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
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
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

        const char* ice_id() const noexcept override;

        void ice_print(std::ostream& stream) const override;

        /**
         * The stringified proxy for which no suitable endpoint is available.
         */
        std::string proxy;
    };

    /**
     * This exception is raised if an illegal identity is encountered.
     * \headerfile Ice/Ice.h
     */
    class ICE_API IllegalIdentityException : public LocalException
    {
    public:
        using LocalException::LocalException;

        const char* ice_id() const noexcept override;

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
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
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

        const char* ice_id() const noexcept override;

        void ice_print(std::ostream& stream) const override;

        /**
         * Describes why this servant is illegal.
         */
        std::string reason;
    };

    /**
     * This exception is raised if a system error occurred in the server or client process. There are many possible
     * causes for such a system exception. For details on the cause, {@link SyscallException#error} should be inspected.
     * \headerfile Ice/Ice.h
     */
    class ICE_API SyscallException : public LocalException
    {
    public:
        /**
         * Construct a SyscallException.
         * The file and line number are required for all local exceptions.
         * The error code is filled automatically with the value of the current error code.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         */
        SyscallException(const char* file, int line) noexcept;

        /**
         * One-shot constructor to initialize all data members.
         * The file and line number are required for all local exceptions.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         * @param error The error number describing the system exception.
         */
        SyscallException(const char* file, int line, int error) noexcept : LocalException(file, line), error(error) {}

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const int&> ice_tuple() const noexcept { return std::tie(error); }

        const char* ice_id() const noexcept override;

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

        const char* ice_id() const noexcept override;

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
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
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

        const char* ice_id() const noexcept override;

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
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
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

        const char* ice_id() const noexcept override;

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

        const char* ice_id() const noexcept override;

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

        const char* ice_id() const noexcept override;

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

        const char* ice_id() const noexcept override;

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
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
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

        const char* ice_id() const noexcept override;

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

        const char* ice_id() const noexcept override;

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

        const char* ice_id() const noexcept override;

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

        const char* ice_id() const noexcept override;

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

        const char* ice_id() const noexcept override;

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

        const char* ice_id() const noexcept override;

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

        const char* ice_id() const noexcept override;

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

        const char* ice_id() const noexcept override;

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
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
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

        const char* ice_id() const noexcept override;

        void ice_print(std::ostream& stream) const override;

        /**
         * True if the connection was closed gracefully, false otherwise.
         */
        bool graceful;
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
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
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

        const char* ice_id() const noexcept override;

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

        const char* ice_id() const noexcept override;

        void ice_print(std::ostream& stream) const override;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
