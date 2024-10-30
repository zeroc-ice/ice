//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CONNECTION_H
#define ICE_CONNECTION_H

#include "Config.h"
#include "ConnectionF.h"
#include "EndpointF.h"
#include "Ice/Identity.h"
#include "Ice/Proxy.h"
#include "Ice/ProxyFunctions.h"
#include "ObjectAdapterF.h"

#include <future>
#include <map>
#include <optional>

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
     * The batch compression option when flushing queued batch requests.
     */
    enum class CompressBatch : std::uint8_t
    {
        /**
         * Compress the batch requests.
         */
        Yes,
        /**
         * Don't compress the batch requests.
         */
        No,
        /**
         * Compress the batch requests if at least one request was made on a compressed proxy.
         */
        BasedOnProxy
    };

    /**
     * A collection of HTTP headers.
     */
    using HeaderDict = std::map<std::string, std::string>;
}

namespace Ice
{
    /**
     * Base class providing access to the connection details.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ConnectionInfo
    {
    public:
        ConnectionInfo() = default;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying transport or null if there's no underlying transport.
         * @param incoming Whether or not the connection is an incoming or outgoing connection.
         * @param adapterName The name of the adapter associated with the connection.
         * @param connectionId The connection id.
         */
        ConnectionInfo(ConnectionInfoPtr underlying, bool incoming, std::string adapterName, std::string connectionId)
            : underlying(std::move(underlying)),
              incoming(incoming),
              adapterName(std::move(adapterName)),
              connectionId(std::move(connectionId))
        {
        }

        virtual ~ConnectionInfo();

        // Deleted to prevent accidental slicing.
        ConnectionInfo(const ConnectionInfo&) = delete;
        ConnectionInfo& operator=(const ConnectionInfo&) = delete;

        /**
         * The information of the underlying transport or null if there's no underlying transport.
         */
        ConnectionInfoPtr underlying;
        /**
         * Whether or not the connection is an incoming or outgoing connection.
         */
        bool incoming;
        /**
         * The name of the adapter associated with the connection.
         */
        std::string adapterName;
        /**
         * The connection id.
         */
        std::string connectionId;
    };

    /**
     * This method is called by the connection when the connection is closed. If the callback needs more information
     * about the closure, it can call {@link Connection#throwException}.
     * @param con The connection that closed.
     */
    using CloseCallback = std::function<void(const ConnectionPtr& con)>;

    /**
     * The user-level interface to a connection.
     * \headerfile Ice/Ice.h
     */
    class ICE_API Connection
    {
    public:
        virtual ~Connection() = 0;

        /**
         * Aborts this connection.
         */
        virtual void abort() noexcept = 0;

        /**
         * Starts a graceful closure of this connection once all outstanding invocations have completed.
         * @param response A callback that the implementation calls when the connection is closed gracefully.
         * @param exception A callback that the implementation calls when the connection closure failed. Its
         * exception_ptr parameter is always non-null and describes the reason for the closure.
         * @remarks The response and exception callbacks may be called synchronously (from the calling thread); in
         * particular, this occurs when you call close on a connection that is already closed. The implementation always
         * calls one of the two callbacks once; it never calls both.
         * If closing the connection takes longer than the configured close timeout, the connection is aborted with a
         * CloseTimeoutException.
         */
        virtual void
        close(std::function<void()> response, std::function<void(std::exception_ptr)> exception) noexcept = 0;

        /**
         * Starts a graceful closure of this connection once all outstanding invocations have completed.
         * @return A future that completes then the connection is closed.
         */
        [[nodiscard]] std::future<void> close();

        /**
         * Create a special proxy that always uses this connection. This can be used for callbacks from a server to a
         * client if the server cannot directly establish a connection to the client, for example because of firewalls.
         * In this case, the server would create a proxy using an already established connection from the client.
         * @param id The identity for which a proxy is to be created.
         * @return A proxy that matches the given identity and uses this connection.
         * @see #setAdapter
         */
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
        Prx createProxy(Identity id) const
        {
            return uncheckedCast<Prx>(_createProxy(std::move(id)));
        }

        /**
         * Associates an object adapter with this connection. When a connection receives a request, it dispatches this
         * request using its associated object adapter. If the associated object adapter is null, the connection
         * rejects any incoming request with an ObjectNotExistException.
         * The default object adapter of an incoming connection is the object adapter that created this connection;
         * the default object adapter of an outgoing connection is the communicator's default object adapter.
         * @param adapter The object adapter to associate with this connection.
         * @see Communicator#getDefaultObjectAdapter
         * @see #getAdapter
         */
        virtual void setAdapter(const ObjectAdapterPtr& adapter) = 0;

        /**
         * Gets the object adapter associated with this connection.
         * @return The object adapter associated with this connection.
         * @see #setAdapter
         */
        virtual ObjectAdapterPtr getAdapter() const noexcept = 0;

        /**
         * Get the endpoint from which the connection was created.
         * @return The endpoint from which the connection was created.
         */
        virtual EndpointPtr getEndpoint() const noexcept = 0;

        /**
         * Flush any pending batch requests for this connection. This means all batch requests invoked on fixed proxies
         * associated with the connection.
         * @param compress Specifies whether or not the queued batch requests should be compressed before being sent
         * over the wire.
         */
        void flushBatchRequests(CompressBatch compress);

        /**
         * Flush any pending batch requests for this connection. This means all batch requests invoked on fixed proxies
         * associated with the connection.
         * @param compress Specifies whether or not the queued batch requests should be compressed before being sent
         * over the wire.
         * @param exception The exception callback.
         * @param sent The sent callback.
         * @return A function that can be called to cancel the invocation locally.
         */
        virtual std::function<void()> flushBatchRequestsAsync(
            CompressBatch compress,
            std::function<void(std::exception_ptr)> exception,
            std::function<void(bool)> sent = nullptr) = 0;

        /**
         * Flush any pending batch requests for this connection. This means all batch requests invoked on fixed proxies
         * associated with the connection.
         * @param compress Specifies whether or not the queued batch requests should be compressed before being sent
         * over the wire.
         * @return The future object for the invocation.
         */
        [[nodiscard]] std::future<void> flushBatchRequestsAsync(CompressBatch compress);

        /**
         * Set a close callback on the connection. The callback is called by the connection when it's closed. The
         * callback is called from the Ice thread pool associated with the connection. If the callback needs more
         * information about the closure, it can call {@link Connection#throwException}.
         * @param callback The close callback object.
         */
        virtual void setCloseCallback(CloseCallback callback) = 0;

        /**
         * Return the connection type. This corresponds to the endpoint type, i.e., "tcp", "udp", etc.
         * @return The type of the connection.
         */
        virtual std::string type() const noexcept = 0;

        /**
         * Return a description of the connection as human readable text, suitable for logging or error messages.
         * @return The description of the connection as human readable text.
         */
        virtual std::string toString() const noexcept = 0;

        /**
         * Returns the connection information.
         * @return The connection information.
         */
        virtual ConnectionInfoPtr getInfo() const = 0;

        /**
         * Set the connection buffer receive/send size.
         * @param rcvSize The connection receive buffer size.
         * @param sndSize The connection send buffer size.
         */
        virtual void setBufferSize(int rcvSize, int sndSize) = 0;

        /**
         * Throw an exception indicating the reason for connection closure. For example,
         * {@link CloseConnectionException} is raised if the connection was closed gracefully by the peer, whereas
         * {@link ConnectionAbortedException} or {@link ConnectionClosedException} is raised if the connection was
         * manually closed by the application. This operation does nothing if the connection is not yet closed.
         */
        virtual void throwException() const = 0;

    protected:
        virtual ObjectPrx _createProxy(Identity id) const = 0;
    };

    /**
     * Provides access to the connection details of an IP connection
     * \headerfile Ice/Ice.h
     */
    class ICE_API IPConnectionInfo : public ConnectionInfo
    {
    public:
        IPConnectionInfo() : localAddress(""), localPort(-1), remoteAddress(""), remotePort(-1) {}

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying transport or null if there's no underlying transport.
         * @param incoming Whether or not the connection is an incoming or outgoing connection.
         * @param adapterName The name of the adapter associated with the connection.
         * @param connectionId The connection id.
         * @param localAddress The local address.
         * @param localPort The local port.
         * @param remoteAddress The remote address.
         * @param remotePort The remote port.
         */
        IPConnectionInfo(
            ConnectionInfoPtr underlying,
            bool incoming,
            std::string adapterName,
            std::string connectionId,
            std::string localAddress,
            int localPort,
            std::string remoteAddress,
            int remotePort)
            : ConnectionInfo(std::move(underlying), incoming, std::move(adapterName), std::move(connectionId)),
              localAddress(std::move(localAddress)),
              localPort(localPort),
              remoteAddress(std::move(remoteAddress)),
              remotePort(remotePort)
        {
        }

        ~IPConnectionInfo() override;

        // Deleted to prevent accidental slicing.
        IPConnectionInfo(const IPConnectionInfo&) = delete;
        IPConnectionInfo& operator=(const IPConnectionInfo&) = delete;

        /**
         * The local address.
         */
        std::string localAddress;
        /**
         * The local port.
         */
        int localPort = -1;
        /**
         * The remote address.
         */
        std::string remoteAddress;
        /**
         * The remote port.
         */
        int remotePort = -1;
    };

    /**
     * Provides access to the connection details of a TCP connection
     * \headerfile Ice/Ice.h
     */
    class ICE_API TCPConnectionInfo : public IPConnectionInfo
    {
    public:
        TCPConnectionInfo() : rcvSize(0), sndSize(0) {}

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying transport or null if there's no underlying transport.
         * @param incoming Whether or not the connection is an incoming or outgoing connection.
         * @param adapterName The name of the adapter associated with the connection.
         * @param connectionId The connection id.
         * @param localAddress The local address.
         * @param localPort The local port.
         * @param remoteAddress The remote address.
         * @param remotePort The remote port.
         * @param rcvSize The connection buffer receive size.
         * @param sndSize The connection buffer send size.
         */
        TCPConnectionInfo(
            ConnectionInfoPtr underlying,
            bool incoming,
            std::string adapterName,
            std::string connectionId,
            std::string localAddress,
            int localPort,
            std::string remoteAddress,
            int remotePort,
            int rcvSize,
            int sndSize)
            : IPConnectionInfo(
                  std::move(underlying),
                  incoming,
                  std::move(adapterName),
                  std::move(connectionId),
                  std::move(localAddress),
                  localPort,
                  std::move(remoteAddress),
                  remotePort),
              rcvSize(rcvSize),
              sndSize(sndSize)
        {
        }

        ~TCPConnectionInfo() override;

        // Deleted to prevent accidental slicing.
        TCPConnectionInfo(const TCPConnectionInfo&) = delete;
        TCPConnectionInfo& operator=(const TCPConnectionInfo&) = delete;

        /**
         * The connection buffer receive size.
         */
        int rcvSize = 0;
        /**
         * The connection buffer send size.
         */
        int sndSize = 0;
    };

    /**
     * Provides access to the connection details of a UDP connection
     * \headerfile Ice/Ice.h
     */
    class ICE_API UDPConnectionInfo : public IPConnectionInfo
    {
    public:
        UDPConnectionInfo() : mcastPort(-1), rcvSize(0), sndSize(0) {}

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying transport or null if there's no underlying transport.
         * @param incoming Whether or not the connection is an incoming or outgoing connection.
         * @param adapterName The name of the adapter associated with the connection.
         * @param connectionId The connection id.
         * @param localAddress The local address.
         * @param localPort The local port.
         * @param remoteAddress The remote address.
         * @param remotePort The remote port.
         * @param mcastAddress The multicast address.
         * @param mcastPort The multicast port.
         * @param rcvSize The connection buffer receive size.
         * @param sndSize The connection buffer send size.
         */
        UDPConnectionInfo(
            ConnectionInfoPtr underlying,
            bool incoming,
            std::string adapterName,
            std::string connectionId,
            std::string localAddress,
            int localPort,
            std::string remoteAddress,
            int remotePort,
            std::string mcastAddress,
            int mcastPort,
            int rcvSize,
            int sndSize)
            : IPConnectionInfo(
                  std::move(underlying),
                  incoming,
                  std::move(adapterName),
                  std::move(connectionId),
                  std::move(localAddress),
                  localPort,
                  std::move(remoteAddress),
                  remotePort),
              mcastAddress(std::move(mcastAddress)),
              mcastPort(mcastPort),
              rcvSize(rcvSize),
              sndSize(sndSize)
        {
        }

        ~UDPConnectionInfo() override;

        // Deleted to prevent accidental slicing.
        UDPConnectionInfo(const UDPConnectionInfo&) = delete;
        UDPConnectionInfo& operator=(const UDPConnectionInfo&) = delete;

        /**
         * The multicast address.
         */
        std::string mcastAddress;
        /**
         * The multicast port.
         */
        int mcastPort = -1;
        /**
         * The connection buffer receive size.
         */
        int rcvSize = 0;
        /**
         * The connection buffer send size.
         */
        int sndSize = 0;
    };

    /**
     * Provides access to the connection details of a WebSocket connection
     * \headerfile Ice/Ice.h
     */
    class ICE_API WSConnectionInfo : public ConnectionInfo
    {
    public:
        WSConnectionInfo() = default;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying transport or null if there's no underlying transport.
         * @param incoming Whether or not the connection is an incoming or outgoing connection.
         * @param adapterName The name of the adapter associated with the connection.
         * @param connectionId The connection id.
         * @param headers The headers from the HTTP upgrade request.
         */
        WSConnectionInfo(
            ConnectionInfoPtr underlying,
            bool incoming,
            std::string adapterName,
            std::string connectionId,
            HeaderDict headers)
            : ConnectionInfo(std::move(underlying), incoming, std::move(adapterName), std::move(connectionId)),
              headers(std::move(headers))
        {
        }

        ~WSConnectionInfo() override;

        // Deleted to prevent accidental slicing.
        WSConnectionInfo(const WSConnectionInfo&) = delete;
        WSConnectionInfo& operator=(const WSConnectionInfo&) = delete;

        /**
         * The headers from the HTTP upgrade request.
         */
        HeaderDict headers;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
