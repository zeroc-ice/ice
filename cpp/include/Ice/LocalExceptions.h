// Copyright (c) ZeroC, Inc.

#ifndef ICE_LOCAL_EXCEPTIONS_H
#define ICE_LOCAL_EXCEPTIONS_H

#include "Ice/Identity.h"
#include "Ice/ReplyStatus.h"
#include "LocalException.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

// This file contains most exceptions derived from Ice::LocalException.

namespace Ice
{
    class ObjectPrx;

    //
    // Dispatch exceptions
    //

    /// The exception that is thrown when a dispatch failed. This is the base class for local exceptions that can be
    /// marshaled and transmitted "over the wire".
    /// You can throw this exception in the implementation of an operation, or in a middleware. The Ice runtime then
    /// logically rethrows this exception to the client.
    /// @headerfile Ice/Ice.h
    class ICE_API DispatchException : public LocalException
    {
    public:
        /// Constructs a DispatchException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param replyStatus The reply status. It must be greater than ReplyStatus::UserException.
        /// @param message The error message. If nullopt, this constructor creates a default message.
        DispatchException(
            const char* file,
            int line,
            ReplyStatus replyStatus,
            std::optional<std::string> message = std::nullopt);

        /// Gets the reply status.
        /// @return The reply status.
        [[nodiscard]] ReplyStatus replyStatus() const noexcept { return _replyStatus; }

        [[nodiscard]] const char* ice_id() const noexcept override;

    private:
        ReplyStatus _replyStatus;
    };

    /// The base class for the 3 NotExist exceptions.
    /// @headerfile Ice/Ice.h
    class ICE_API RequestFailedException : public DispatchException
    {
    public:
        /// The identity of the Ice Object to which the request was sent.
        [[nodiscard]] const Identity& id() const noexcept { return *_id; }

        /// The facet to which the request was sent.
        [[nodiscard]] const std::string& facet() const noexcept { return *_facet; }

        /// The operation name of the request.
        [[nodiscard]] const std::string& operation() const noexcept { return *_operation; }

    protected:
        /// @cond INTERNAL
        /// Constructs a RequestFailedException with a custom error message.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param replyStatus The reply status.
        /// @param id The identity of the Ice Object to which the request was sent.
        /// @param facet The facet to which the request was sent.
        /// @param operation The operation name of the request.
        RequestFailedException(
            const char* file,
            int line,
            ReplyStatus replyStatus,
            Identity id,
            std::string facet,
            std::string operation);

        /// Constructs a RequestFailedException without specifying the details of the current request. The details will
        /// be filled-in automatically by the Ice runtime before marshaling the exception.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param replyStatus The reply status.
        RequestFailedException(const char* file, int line, ReplyStatus replyStatus);

        RequestFailedException(const RequestFailedException&) noexcept = default;
        RequestFailedException& operator=(const RequestFailedException&) noexcept = default;
        ~RequestFailedException() override; // to avoid weak vtable
        /// @endcond

    private:
        std::shared_ptr<Identity> _id;
        std::shared_ptr<std::string> _facet;
        std::shared_ptr<std::string> _operation;
    };

    /// The exception that is thrown when a dispatch cannot find a servant for the identity carried by the request.
    /// @headerfile Ice/Ice.h
    class ICE_API ObjectNotExistException final : public RequestFailedException
    {
    public:
        /// Constructs an ObjectNotExistException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param id The identity of the Ice Object to which the request was sent.
        /// @param facet The facet to which the request was sent.
        /// @param operation The operation name of the request.
        ObjectNotExistException(const char* file, int line, Identity id, std::string facet, std::string operation);

        /// Constructs an ObjectNotExistException without specifying the details of the current request. The details
        /// will be filled-in automatically by the Ice runtime before marshaling the exception.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        ObjectNotExistException(const char* file, int line);

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when a dispatch cannot find a servant for the identity + facet carried by the
    /// request.
    /// @headerfile Ice/Ice.h
    class ICE_API FacetNotExistException final : public RequestFailedException
    {
    public:
        /// Constructs a FacetNotExistException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param id The identity of the Ice Object to which the request was sent.
        /// @param facet The facet to which the request was sent.
        /// @param operation The operation name of the request.
        FacetNotExistException(const char* file, int line, Identity id, std::string facet, std::string operation);

        /// Constructs a FacetNotExistException without specifying the details of the current request. The details will
        /// be filled-in automatically by the Ice runtime before marshaling the exception.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        FacetNotExistException(const char* file, int line);

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when a dispatch cannot find the operation carried by the request on the target
    /// servant. This is typically due to a mismatch in the Slice definitions, such as the client using Slice
    /// definitions newer than the server's.
    /// @headerfile Ice/Ice.h
    class ICE_API OperationNotExistException final : public RequestFailedException
    {
    public:
        /// Constructs an OperationNotExistException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param id The identity of the Ice Object to which the request was sent.
        /// @param facet The facet to which the request was sent.
        /// @param operation The operation name of the request.
        OperationNotExistException(const char* file, int line, Identity id, std::string facet, std::string operation);

        /// Constructs an OperationNotExistException without specifying the details of the current request. The details
        /// will be filled-in automatically by the Ice runtime before marshaling the exception.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        OperationNotExistException(const char* file, int line);

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when a dispatch failed with an exception that is not a LocalException or a
    /// UserException.
    /// @headerfile Ice/Ice.h
    class ICE_API UnknownException : public DispatchException
    {
    public:
        /// Constructs an UnknownException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param message The error message adopted by this exception and returned by what().
        UnknownException(const char* file, int line, std::string message);

        [[nodiscard]] const char* ice_id() const noexcept override;

    protected:
        /// Constructs an UnknownException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param replyStatus The reply status.
        /// @param message The error message adopted by this exception and returned by what().
        UnknownException(const char* file, int line, ReplyStatus replyStatus, std::string message);
    };

    /// The exception that is thrown when a dispatch failed with a LocalException that is not a DispatchException.
    /// @headerfile Ice/Ice.h
    class ICE_API UnknownLocalException final : public UnknownException
    {
    public:
        /// Constructs an UnknownLocalException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param message The error message adopted by this exception and returned by what().
        UnknownLocalException(const char* file, int line, std::string message);

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when a client receives a UserException that was not declared in the operation's
    /// exception specification.
    /// @headerfile Ice/Ice.h
    class ICE_API UnknownUserException final : public UnknownException
    {
    public:
        /// Constructs an UnknownUserException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param message The error message adopted by this exception and returned by what().
        UnknownUserException(const char* file, int line, std::string message);

        /// Creates an UnknownUserException from the type ID of an unexpected exception.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param typeId The type ID.
        static UnknownUserException fromTypeId(const char* file, int line, const char* typeId);

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    //
    // Protocol exceptions
    //

    /// The base class for exceptions related to the Ice protocol.
    /// @headerfile Ice/Ice.h
    class ICE_API ProtocolException : public LocalException
    {
    public:
        using LocalException::LocalException;

        [[nodiscard]] const char* ice_id() const noexcept override;
    };

    /// The exception that is thrown when the connection has been gracefully shut down by the server. The request
    /// that returned this exception has not been executed by the server. In most cases you will not get this exception,
    /// because the client will automatically retry the invocation. However, if upon retry the server shuts down the
    /// connection again, and the retry limit has been reached, then this exception is propagated to the application
    /// code.
    /// @headerfile Ice/Ice.h
    class ICE_API CloseConnectionException final : public ProtocolException
    {
    public:
        /// Constructs a CloseConnectionException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        CloseConnectionException(const char* file, int line)
            : ProtocolException(file, line, "connection closed by the peer")
        {
        }

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when a datagram exceeds the configured send or receive buffer size, or exceeds the
    /// maximum payload size of a UDP packet (65507 bytes).
    /// @headerfile Ice/Ice.h
    class ICE_API DatagramLimitException final : public ProtocolException
    {
    public:
        /// Constructs a DatagramLimitException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        DatagramLimitException(const char* file, int line)
            : ProtocolException(file, line, "datagram size exceeds configured limit")
        {
        }

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when an error occurs during marshaling or unmarshaling.
    /// @headerfile Ice/Ice.h
    class ICE_API MarshalException final : public ProtocolException
    {
    public:
        using ProtocolException::ProtocolException;

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    //
    // Timeout exceptions
    //

    /// The exception that is thrown when a timeout occurs. This is the base class for all timeout exceptions.
    /// @headerfile Ice/Ice.h
    class ICE_API TimeoutException : public LocalException
    {
    public:
        using LocalException::LocalException;

        [[nodiscard]] const char* ice_id() const noexcept override;
    };

    /// The exception that is thrown when a connection establishment times out.
    /// @headerfile Ice/Ice.h
    class ICE_API ConnectTimeoutException final : public TimeoutException
    {
    public:
        /// Constructs a ConnectTimeoutException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        ConnectTimeoutException(const char* file, int line)
            : TimeoutException(file, line, "connection establishment timed out")
        {
        }

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when a graceful connection closure times out.
    /// @headerfile Ice/Ice.h
    class ICE_API CloseTimeoutException final : public TimeoutException
    {
    public:
        /// Constructs a CloseTimeoutException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        CloseTimeoutException(const char* file, int line)
            : TimeoutException(file, line, "graceful connection closure timed out")
        {
        }

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when an invocation times out.
    /// @headerfile Ice/Ice.h
    class ICE_API InvocationTimeoutException final : public TimeoutException
    {
    public:
        /// Constructs an InvocationTimeoutException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        InvocationTimeoutException(const char* file, int line) : TimeoutException(file, line, "invocation timed out") {}

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    //
    // Syscall exceptions
    //

    /// The exception that is thrown to report the failure of a system call.
    /// @headerfile Ice/Ice.h
    class ICE_API SyscallException : public LocalException
    {
    public:
        /// The error code is a DWORD (unsigned long) on Windows and an int on other platforms.
#ifdef _WIN32
        using ErrorCode = unsigned long;
#else
        using ErrorCode = int;
#endif

        /// Constructs a SyscallException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param messagePrefix The start of the message returned by what().
        /// @param error The error code.
        SyscallException(const char* file, int line, std::string messagePrefix, ErrorCode error);

        /// Gets the error number describing the system exception. On Unix, this is equivalent to
        /// <code>errno</code>. On Windows, this is the value returned by <code>GetLastError()</code> or
        /// <code>WSAGetLastError()</code>.
        [[nodiscard]] ErrorCode error() const noexcept { return _error; }

        [[nodiscard]] const char* ice_id() const noexcept override;

    protected:
        /// Constructs a SyscallException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param messagePrefix The start of the message returned by what().
        /// @param error The error code.
        /// @param errorToString A function that converts the error code to a string.
        SyscallException(
            const char* file,
            int line,
            std::string messagePrefix,
            ErrorCode error,
            const std::function<std::string(ErrorCode)>& errorToString);

        /// Constructs a SyscallException without an error.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param message The message returned by what().
        SyscallException(const char* file, int line, std::string message)
            : LocalException(file, line, std::move(message)),
              _error(0)
        {
        }

    private:
        ErrorCode _error;
    };

    /// The exception that is thrown to report a DNS resolution failure.
    /// @headerfile Ice/Ice.h
    class ICE_API DNSException final : public SyscallException
    {
    public:
        /// Constructs a DNSException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param error The error code.
        /// @param host The host name that could not be resolved.
        DNSException(const char* file, int line, ErrorCode error, std::string_view host);

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown to report a file system error.
    /// @headerfile Ice/Ice.h
    class ICE_API FileException final : public SyscallException
    {
    public:
        /// Constructs a FileException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param path The path of the file that caused the exception.
        /// @param error The error code.
        FileException(const char* file, int line, std::string_view path, ErrorCode error = 0);

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    //
    // Socket exceptions
    //

    /// The exception that is thrown to report a socket error.
    /// @headerfile Ice/Ice.h
    class ICE_API SocketException : public SyscallException
    {
    public:
        /// Constructs a SocketException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param messagePrefix The start of the message returned by what().
        /// @param error The error code.
        SocketException(const char* file, int line, std::string messagePrefix, ErrorCode error);

        /// Constructs a SocketException with a generic message.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param error The error code.
        SocketException(const char* file, int line, ErrorCode error)
            : SocketException(file, line, "socket error", error)
        {
        }

        /// Constructs a SocketException without an error.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param message The message returned by what().
        SocketException(const char* file, int line, std::string message)
            : SyscallException(file, line, std::move(message))
        {
        }

        [[nodiscard]] const char* ice_id() const noexcept override;

    protected:
        using SyscallException::SyscallException;
    };

    /// The exception that is thrown when a connection establishment fails.
    /// @headerfile Ice/Ice.h
    class ICE_API ConnectFailedException : public SocketException
    {
    public:
        using SocketException::SocketException;

        [[nodiscard]] const char* ice_id() const noexcept override;
    };

    /// The exception that is thrown when an established connection is lost.
    /// @headerfile Ice/Ice.h
    class ICE_API ConnectionLostException final : public SocketException
    {
    public:
        /// Constructs a ConnectionLostException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param error The error code.
        ConnectionLostException(const char* file, int line, ErrorCode error);

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when the server host actively refuses a connection.
    /// @headerfile Ice/Ice.h
    class ICE_API ConnectionRefusedException final : public ConnectFailedException
    {
    public:
        /// Constructs a ConnectionRefusedException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        ConnectionRefusedException(const char* file, int line)
            : ConnectFailedException(file, line, "connection refused")
        {
        }

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    //
    // Other leaf local exceptions in alphabetical order.
    //

    /// The exception that is thrown when you attempt to register an object more than once with the Ice runtime.
    /// @headerfile Ice/Ice.h
    class ICE_API AlreadyRegisteredException final : public LocalException
    {
    public:
        /// Constructs a AlreadyRegisteredException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param kindOfObject The kind of object that is already registered.
        /// @param id The ID (or name) of the object that is already registered.
        AlreadyRegisteredException(const char* file, int line, std::string kindOfObject, std::string id);

        /// Gets the kind of object that is already registered: "servant", "facet", "object", "default servant",
        /// "servant locator", "plugin", "object adapter", "object adapter with router", "replica group".
        [[nodiscard]] const std::string& kindOfObject() const noexcept { return *_kindOfObject; }

        /// Gets the ID (or name) of the object that is already registered.
        [[nodiscard]] const std::string& id() const noexcept { return *_id; }

        [[nodiscard]] const char* ice_id() const noexcept final;

    private:
        std::shared_ptr<std::string> _kindOfObject;
        std::shared_ptr<std::string> _id;
    };

    /// The exception that is thrown when an operation fails because the communicator has been destroyed.
    /// @see Communicator#destroy
    /// @headerfile Ice/Ice.h
    class ICE_API CommunicatorDestroyedException final : public LocalException
    {
    public:
        /// Constructs a CommunicatorDestroyedException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        CommunicatorDestroyedException(const char* file, int line)
            : LocalException(file, line, "the Ice communicator object is destroyed")
        {
        }

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when an operation fails because the connection has been aborted.
    /// @headerfile Ice/Ice.h
    class ICE_API ConnectionAbortedException final : public LocalException
    {
    public:
        /// Constructs a ConnectionAbortedException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param message The message returned by what().
        /// @param closedByApplication True if the connection was aborted by the application, false if the connection
        /// was aborted by the Ice runtime.
        ConnectionAbortedException(const char* file, int line, std::string message, bool closedByApplication)
            : LocalException(file, line, std::move(message)),
              _closedByApplication(closedByApplication)
        {
        }

        /// Returns true if the connection was aborted by the application, false if it was aborted by the Ice runtime.
        [[nodiscard]] bool closedByApplication() const noexcept { return _closedByApplication; }

        [[nodiscard]] const char* ice_id() const noexcept final;

    private:
        bool _closedByApplication;
    };

    /// The exception that is thrown when an operation fails because the connection has been closed gracefully.
    /// @headerfile Ice/Ice.h
    class ICE_API ConnectionClosedException final : public LocalException
    {
    public:
        /// Constructs a ConnectionClosedException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param message The message returned by what().
        /// @param closedByApplication True if the connection was closed by the application, false if the connection was
        /// closed by the Ice runtime.
        ConnectionClosedException(const char* file, int line, std::string message, bool closedByApplication)
            : LocalException(file, line, std::move(message)),
              _closedByApplication(closedByApplication)
        {
        }

        /// Returns true if the connection was closed by the application, false if it was closed by the Ice runtime.
        [[nodiscard]] bool closedByApplication() const noexcept { return _closedByApplication; }

        [[nodiscard]] const char* ice_id() const noexcept final;

    private:
        bool _closedByApplication;
    };

    /// The exception that is thrown when attempting to use an unsupported feature.
    /// @headerfile Ice/Ice.h
    class ICE_API FeatureNotSupportedException final : public LocalException
    {
    public:
        using LocalException::LocalException;

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when attempting to change a connection-related property on a fixed proxy.
    /// @headerfile Ice/Ice.h
    class ICE_API FixedProxyException final : public LocalException
    {
    public:
        /// Constructs a FixedProxyException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        FixedProxyException(const char* file, int line)
            : LocalException(file, line, "cannot change the properties of a fixed proxy")
        {
        }

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when communicator initialization fails.
    /// @headerfile Ice/Ice.h
    class ICE_API InitializationException final : public LocalException
    {
    public:
        using LocalException::LocalException;

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when an asynchronous invocation fails because it was canceled explicitly by the
    /// user.
    /// @headerfile Ice/Ice.h
    class ICE_API InvocationCanceledException final : public LocalException
    {
    public:
        /// Constructs an InvocationCanceledException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        InvocationCanceledException(const char* file, int line) : LocalException(file, line, "invocation canceled") {}

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when the Ice runtime cannot find a suitable endpoint to connect to.
    /// @headerfile Ice/Ice.h
    class ICE_API NoEndpointException final : public LocalException
    {
    public:
        using LocalException::LocalException;

        /// Constructs a NoEndpointException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param proxy The proxy used to create the message return by what().
        NoEndpointException(const char* file, int line, const ObjectPrx& proxy);

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when attempting to find or deregister something that is not registered with Ice.
    /// @headerfile Ice/Ice.h
    class ICE_API NotRegisteredException final : public LocalException
    {
    public:
        /// Constructs a NotRegisteredException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param kindOfObject The kind of object that is not registered.
        /// @param id The ID (or name) of the object that is not registered.
        NotRegisteredException(const char* file, int line, std::string kindOfObject, std::string id);

        /// Gets the kind of object that is not registered.
        [[nodiscard]] const std::string& kindOfObject() const noexcept { return *_kindOfObject; }

        /// Gets the ID (or name) of the object that is not registered.
        [[nodiscard]] const std::string& id() const noexcept { return *_id; };

        [[nodiscard]] const char* ice_id() const noexcept final;

    private:
        std::shared_ptr<std::string> _kindOfObject;
        std::shared_ptr<std::string> _id;
    };

    /// The exception that is thrown when attempting to use an ObjectAdapter that has been deactivated.
    /// @see ObjectAdapter#deactivate
    /// @see Communicator#shutdown
    /// @headerfile Ice/Ice.h
    class ICE_API ObjectAdapterDeactivatedException final : public LocalException
    {
    public:
        /// Constructs an ObjectAdapterDeactivatedException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param name The name of the object adapter that is deactivated.
        ObjectAdapterDeactivatedException(const char* file, int line, std::string_view name)
            : LocalException(file, line, "object adapter '" + std::string{name} + "' is deactivated")
        {
        }

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when attempting to use an ObjectAdapter that has been destroyed.
    /// @headerfile Ice/Ice.h
    class ICE_API ObjectAdapterDestroyedException final : public LocalException
    {
    public:
        /// Constructs an ObjectAdapterDestroyedException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param name The name of the object adapter that is destroyed.
        ObjectAdapterDestroyedException(const char* file, int line, std::string_view name)
            : LocalException(file, line, "object adapter '" + std::string{name} + "' is destroyed")
        {
        }

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when an ObjectAdapter cannot be activated. This can happen when the locator
    /// implementation detects another active ObjectAdapter with the same adapter ID.
    /// @headerfile Ice/Ice.h
    class ICE_API ObjectAdapterIdInUseException final : public LocalException
    {
    public:
        /// Constructs an ObjectAdapterIdInUseException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param adapterId The adapter ID used to create the message returned by what().
        ObjectAdapterIdInUseException(const char* file, int line, std::string_view adapterId)
            : LocalException(file, line, "an object adapter with ID '" + std::string{adapterId} + "' is already active")
        {
        }

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when the parsing of a string fails.
    /// @headerfile Ice/Ice.h
    class ICE_API ParseException final : public LocalException
    {
    public:
        using LocalException::LocalException;

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when the initialization of an Ice plugin fails.
    /// @headerfile Ice/Ice.h
    class ICE_API PluginInitializationException final : public LocalException
    {
    public:
        using LocalException::LocalException;

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when a failure occurs in the security subsystem. This includes IceSSL errors.
    /// @headerfile Ice/Ice.h
    class ICE_API SecurityException final : public LocalException
    {
    public:
        using LocalException::LocalException;

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when attempting to invoke an operation with `ice_oneway`, `ice_batchOneway`,
    /// `ice_datagram`, or `ice_batchDatagram`, and the operation has a return value, an out parameter, or an exception
    /// specification.
    /// @headerfile Ice/Ice.h
    class ICE_API TwowayOnlyException final : public LocalException
    {
    public:
        /// Constructs a TwowayOnlyException.
        /// @param file The file where this exception is constructed. This C string is not copied.
        /// @param line The line where this exception is constructed.
        /// @param operation The two-way only operation used to create the message returned by what().
        TwowayOnlyException(const char* file, int line, std::string_view operation)
            : LocalException(
                  file,
                  line,
                  "cannot invoke operation '" + std::string{operation} +
                      "' with a oneway, batchOneway, datagram, or batchDatagram proxy")
        {
        }

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /// The exception that is thrown when a property cannot be set or retrieved. For example, this exception is thrown
    /// when attempting to set an unknown Ice property.
    class ICE_API PropertyException final : public LocalException
    {
    public:
        using LocalException::LocalException;

        [[nodiscard]] const char* ice_id() const noexcept final;
    };
}

#endif
