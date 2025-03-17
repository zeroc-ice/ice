// Copyright (c) ZeroC, Inc.

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
        virtual ~Connection();

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
        template<typename Prx = ObjectPrx, std::enable_if_t<std::is_base_of_v<ObjectPrx, Prx>, bool> = true>
        [[nodiscard]] Prx createProxy(Identity id) const
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
        [[nodiscard]] virtual ObjectAdapterPtr getAdapter() const noexcept = 0;

        /**
         * Get the endpoint from which the connection was created.
         * @return The endpoint from which the connection was created.
         */
        [[nodiscard]] virtual EndpointPtr getEndpoint() const noexcept = 0;

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
        [[nodiscard]] virtual const std::string& type() const noexcept = 0;

        /**
         * Return a description of the connection as human readable text, suitable for logging or error messages.
         * @return The description of the connection as human readable text.
         * @remark This function remains usable after the connection is closed or aborted.
         */
        [[nodiscard]] virtual std::string toString() const = 0;

        /**
         * Returns the connection information.
         * @return The connection information.
         */
        [[nodiscard]] virtual ConnectionInfoPtr getInfo() const = 0;

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
        /// @private
        [[nodiscard]] virtual ObjectPrx _createProxy(Identity id) const = 0;
    };

    /**
     * Base class providing access to the connection details.
     * \headerfile Ice/Ice.h
     */
    class ICE_API ConnectionInfo
    {
    public:
        virtual ~ConnectionInfo();

        // Deleted to prevent accidental slicing.
        ConnectionInfo(const ConnectionInfo&) = delete;
        ConnectionInfo& operator=(const ConnectionInfo&) = delete;

        /**
         * The information of the underlying transport or null if there's no underlying transport.
         */
        const ConnectionInfoPtr underlying;

        /**
         * Whether or not the connection is an incoming or outgoing connection.
         */
        const bool incoming;

        /**
         * The name of the adapter associated with the connection.
         */
        const std::string adapterName;

        /**
         * The connection id.
         */
        const std::string connectionId;

    protected:
        /// @private
        explicit ConnectionInfo(ConnectionInfoPtr underlyingInfo)
            : underlying{std::move(underlyingInfo)},
              incoming{underlying->incoming},
              adapterName{underlying->adapterName},
              connectionId{underlying->connectionId}
        {
        }

        /// @private
        ConnectionInfo(bool incoming, std::string adapterName, std::string connectionId)
            : incoming{incoming},
              adapterName{std::move(adapterName)},
              connectionId{std::move(connectionId)}
        {
        }
    };

    /**
     * Provides access to the connection details of an IP connection
     * \headerfile Ice/Ice.h
     */
    class ICE_API IPConnectionInfo : public ConnectionInfo
    {
    public:
        IPConnectionInfo(const IPConnectionInfo&) = delete;
        IPConnectionInfo& operator=(const IPConnectionInfo&) = delete;

        ~IPConnectionInfo() override;

        /**
         * The local address.
         */
        const std::string localAddress;

        /**
         * The local port.
         */
        const int localPort;

        /**
         * The remote address.
         */
        const std::string remoteAddress;

        /**
         * The remote port.
         */
        const int remotePort;

    protected:
        /// @private
        IPConnectionInfo(
            bool incoming,
            std::string adapterName,
            std::string connectionId,
            std::string localAddress,
            int localPort,
            std::string remoteAddress,
            int remotePort)
            : ConnectionInfo{incoming, std::move(adapterName), std::move(connectionId)},
              localAddress{std::move(localAddress)},
              localPort{localPort},
              remoteAddress{std::move(remoteAddress)},
              remotePort{remotePort}
        {
        }
    };

    /**
     * Provides access to the connection details of a TCP connection
     * \headerfile Ice/Ice.h
     */
    class ICE_API TCPConnectionInfo final : public IPConnectionInfo
    {
    public:
        ~TCPConnectionInfo() final;
        TCPConnectionInfo(const TCPConnectionInfo&) = delete;
        TCPConnectionInfo& operator=(const TCPConnectionInfo&) = delete;

        /**
         * The connection buffer receive size.
         */
        const int rcvSize;
        /**
         * The connection buffer send size.
         */
        const int sndSize;

        // internal constructor
        /// @private
        TCPConnectionInfo(
            bool incoming,
            std::string adapterName,
            std::string connectionId,
            std::string localAddress,
            int localPort,
            std::string remoteAddress,
            int remotePort,
            int rcvSize,
            int sndSize)
            : IPConnectionInfo{incoming, std::move(adapterName), std::move(connectionId), std::move(localAddress), localPort, std::move(remoteAddress), remotePort},
              rcvSize{rcvSize},
              sndSize{sndSize}
        {
        }

        // internal constructor
        /// @private
        TCPConnectionInfo(bool incoming, std::string adapterName, std::string connectionId)
            : TCPConnectionInfo{incoming, std::move(adapterName), std::move(connectionId), "", -1, "", -1, 0, 0}
        {
        }
    };

    /**
     * Provides access to the connection details of a UDP connection
     * \headerfile Ice/Ice.h
     */
    class ICE_API UDPConnectionInfo final : public IPConnectionInfo
    {
    public:
        ~UDPConnectionInfo() final;
        UDPConnectionInfo(const UDPConnectionInfo&) = delete;
        UDPConnectionInfo& operator=(const UDPConnectionInfo&) = delete;

        /**
         * The multicast address.
         */
        const std::string mcastAddress;

        /**
         * The multicast port.
         */
        const int mcastPort;

        /**
         * The connection buffer receive size.
         */
        const int rcvSize;

        /**
         * The connection buffer send size.
         */
        const int sndSize;

        // internal constructor
        /// @private
        UDPConnectionInfo(
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
            : IPConnectionInfo{incoming, std::move(adapterName), std::move(connectionId), std::move(localAddress), localPort, std::move(remoteAddress), remotePort},
              mcastAddress{std::move(mcastAddress)},
              mcastPort{mcastPort},
              rcvSize{rcvSize},
              sndSize{sndSize}
        {
        }

        // internal constructor
        /// @private
        UDPConnectionInfo(bool incoming, std::string adapterName, std::string connectionId)
            : UDPConnectionInfo{incoming, std::move(adapterName), std::move(connectionId), "", -1, "", -1, "", -1, 0, 0}
        {
        }
    };

    /**
     * Provides access to the connection details of a WebSocket connection
     * \headerfile Ice/Ice.h
     */
    class ICE_API WSConnectionInfo final : public ConnectionInfo
    {
    public:
        ~WSConnectionInfo() final;
        WSConnectionInfo(const WSConnectionInfo&) = delete;
        WSConnectionInfo& operator=(const WSConnectionInfo&) = delete;

        /**
         * The headers from the HTTP upgrade request.
         */
        const HeaderDict headers;

        // internal constructor
        /// @private
        WSConnectionInfo(ConnectionInfoPtr underlying, HeaderDict headers)
            : ConnectionInfo{std::move(underlying)},
              headers{std::move(headers)}
        {
        }
    };

    /**
     * Provides access to the connection details of an IAP connection
     * \headerfile Ice/Ice.h
     */
    class IAPConnectionInfo final : public ConnectionInfo
    {
    public:
        ~IAPConnectionInfo() final;
        IAPConnectionInfo(const IAPConnectionInfo&) = delete;
        IAPConnectionInfo& operator=(const IAPConnectionInfo&) = delete;

        /**
         * The accessory name.
         */
        const std::string name;

        /**
         * The accessory manufacturer.
         */
        const std::string manufacturer;

        /**
         * The accessory model number.
         */
        const std::string modelNumber;

        /**
         * The accessory firmware revision.
         */
        const std::string firmwareRevision;

        /**
         * The accessory hardware revision.
         */
        const std::string hardwareRevision;

        /**
         * The protocol used by the accessory.
         */
        const std::string protocol;

        // internal constructor
        /// @private
        IAPConnectionInfo(
            std::string adapterName,
            std::string connectionId,
            std::string name,
            std::string manufacturer,
            std::string modelNumber,
            std::string firmwareRevision,
            std::string hardwareRevision,
            std::string protocol)
            : ConnectionInfo{false, std::move(adapterName), std::move(connectionId)},
              name{std::move(name)},
              manufacturer{std::move(manufacturer)},
              modelNumber{std::move(modelNumber)},
              firmwareRevision{std::move(firmwareRevision)},
              hardwareRevision{std::move(hardwareRevision)},
              protocol{std::move(protocol)}
        {
        }
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#endif
