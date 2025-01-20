// Copyright (c) ZeroC, Inc.

declare module "ice" {
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
             * @returns A promise that resolves when the close operation is complete.
             */
            close(): Promise<void>;

            /**
             * Creates a special proxy that always uses this connection. This is useful for callbacks from a server to a
             * client when the server cannot directly establish a connection to the client, such as in cases where firewalls
             * are present. In such scenarios, the server would create a proxy using an already established connection from the client.
             *
             * @param id - The identity for which the proxy is to be created.
             * @returns A proxy that matches the given identity and uses this connection.
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
             * @returns The asynchronous result object for the invocation.
             */
            flushBatchRequests(): Promise<void>;

            /**
             * Sets a close callback on the connection. The callback is invoked by the connection when it is closed.
             * If the callback needs more information about the closure, it can call {@link Connection#throwException}.
             *
             * @param callback - The close callback object.
             */
            setCloseCallback(callback: Ice.CloseCallback): void;

            /**
             * Returns the connection type, which corresponds to the endpoint type (e.g., "tcp", "udp", etc.).
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
             */
            getInfo(): Ice.ConnectionInfo;

            /**
             * Sets the connection buffer sizes for receiving and sending data.
             *
             * @param rcvSize - The size of the receive buffer in bytes.
             * @param sndSize - The size of the send buffer in bytes.
             */
            setBufferSize(rcvSize: number, sndSize: number): void;

            /**
             * Throw an exception indicating the reason for connection closure. For example,
             * {@link CloseConnectionException} is raised if the connection was closed gracefully, whereas
             * {@link ConnectionAbortedException}/{@link ConnectionClosedException} is raised if the connection was
             * manually closed by the application. This operation does nothing if the connection is not yet closed.
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
             * The local address.
             */
            get localAddress(): string;

            /**
             * The local port.
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
        class TCPConnectionInfo extends IPConnectionInfo {
            /**
             * The connection buffer send size.
             */
            get sndSize(): number;
        }

        /**
         * Provides access to the connection details of a WebSocket connection.
         */
        class WSConnectionInfo extends ConnectionInfo {}
    }
}
