// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Callback invoked when the connection is closed. If additional information about the closure is needed,
         * the callback can call {@link Connection#throwException}.
         *
         * @param connection - The connection that was closed.
         */
        type CloseCallback = (connection: Ice.Connection) => void;

        /**
         * The user-level interface to a connection.
         */
        interface Connection {
            /**
             * Manually aborts the connection.
             */
            abort(): void;

            /**
             * Manually closes the connection gracefully after waiting for all pending invocations
             * to complete.
             *
             * @returns A promise that resolves when the connection is gracefully closed, and rejects when the
             * connection is lost or the graceful closure times out ({@link CloseTimeoutException}).
             */
            close(): Promise<void>;

            /**
             * Creates a special proxy that always uses this connection. This is useful for callbacks from a server to a
             * client when the server cannot directly establish a connection to the client, such as in cases where firewalls
             * are present. In such scenarios, the server would create a proxy using an already established connection from the client.
             *
             * @param id - The identity for which the proxy is to be created.
             * @returns A proxy that matches the given identity and uses this connection.
             * @throws {@link CommunicatorDestroyedException} - Thrown when the communicator has been destroyed.
             *
             * @see {@link setAdapter}
             */
            createProxy(id: Identity): Ice.ObjectPrx;

            /**
             * Associates an object adapter with this connection. When a connection receives a request, it dispatches
             * this request using its associated object adapter. If the associated object adapter is null, the
             * connection rejects any incoming request with an {@link ObjectNotExistException}. The default object
             * adapter of an outgoing connection is the communicator's default object adapter.
             *
             * @param adapter - The object adapter to associate with this connection.
             *
             * @see {@link createProxy}
             * @see {@link getAdapter}
             */
            setAdapter(adapter: Ice.ObjectAdapter | null): void;

            /**
             * Gets the object adapter associated with this connection.
             *
             * @returns The object adapter associated with this connection.
             *
             * @see {@link setAdapter}
             */
            getAdapter(): Ice.ObjectAdapter | null;

            /**
             * Get the endpoint from which the connection was created.
             *
             * @returns The endpoint from which the connection was created.
             */
            getEndpoint(): Ice.Endpoint;

            /**
             * Flush any pending batch requests for this connection. This means all batch requests invoked on fixed proxies
             * associated with the connection.
             * @returns The asynchronous result object for the invocation. If the flush fails, the promise is
             * rejected with the corresponding exception, for example {@link CommunicatorDestroyedException} when
             * the communicator has been destroyed.
             */
            flushBatchRequests(): Promise<void>;

            /**
             * Sets a close callback on the connection. The callback is invoked by the connection when it is closed.
             * If the callback needs more information about the closure, it can call {@link Connection#throwException}.
             *
             * @param callback - The close callback object. Pass `null` to clear the callback.
             */
            setCloseCallback(callback: Ice.CloseCallback | null): void;

            /**
             * Disables the inactivity check on this connection.
             */
            disableInactivityCheck(): void;

            /**
             * Returns the connection type, which corresponds to the endpoint type (e.g., "tcp", "ws", "wss").
             *
             * @returns The type of the connection.
             */
            type(): string;

            /**
             * Return a description of the connection as human readable text, suitable for logging or error messages.
             *
             * @returns The description of the connection as human readable text.
             */
            toString(): string;

            /**
             * Retrieves the connection information.
             *
             * @returns The connection information.
             * @throws The exception that caused the closure, when this connection is closed.
             */
            getInfo(): Ice.ConnectionInfo;

            /**
             * Throw an exception indicating the reason for connection closure. For example,
             * {@link CloseConnectionException} is raised if the connection was closed gracefully by the peer, whereas
             * {@link ConnectionAbortedException}/{@link ConnectionClosedException} is raised if the connection was
             * manually closed by the application. This operation does nothing if the connection is not yet closing
             * or closed.
             */
            throwException(): void;
        }

        /**
         * Base class providing access to the connection details.
         */
        class ConnectionInfo {
            /**
             * The information of the underlying transport, or `null` if there is no underlying transport.
             */
            get underlying(): Ice.ConnectionInfo | null;

            /**
             * The name of the adapter associated with this connection.
             */
            get adapterName(): string;

            /**
             * The connection id.
             */
            get connectionId(): string;
        }

        /**
         * Provides access to the connection details of an IP connection
         */
        class IPConnectionInfo extends ConnectionInfo {
            /**
             * The local address. For WebSocket connections, the local address is not available and `localAddress`
             * is `""`.
             */
            get localAddress(): string;

            /**
             * The local port. For WebSocket connections, the local port is not available and `localPort` is `-1`.
             */
            get localPort(): number;

            /**
             * The remote address.
             */
            get remoteAddress(): string;

            /**
             * The remote port.
             */
            get remotePort(): number;
        }

        /**
         * Provides access to the connection details of a TCP connection
         */
        class TCPConnectionInfo extends IPConnectionInfo {}

        /**
         * Provides access to the connection details of a WebSocket connection.
         */
        class WSConnectionInfo extends ConnectionInfo {
            /**
             * The maximum number of bytes that can be buffered by a WebSocket connection. Once the WebSocket
             * `bufferedAmount` reaches this limit, sending additional data is delayed until it drops below the
             * threshold.
             */
            get maxBufferedAmount(): number;
        }
    }
}
