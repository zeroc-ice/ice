//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export namespace Ice
{
    /**
     * The batch compression option when flushing queued batch requests.
     */
    class CompressBatch
    {
        /**
         * Compress the batch requests.
         */
        static readonly Yes:CompressBatch;
        /**
         * Don't compress the batch requests.
         */
        static readonly No:CompressBatch;
        /**
         * Compress the batch requests if at least one request was
         * made on a compressed proxy.
         */
        static readonly BasedOnProxy:CompressBatch;
        static valueOf(value:number):CompressBatch;
        equals(other:any):boolean;
        hashCode():number;
        toString():string;
        readonly name:string;
        readonly value:number;
    }

    /**
     * Base class providing access to the connection details.
     */
    class ConnectionInfo
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underyling transport or null if there's no underlying transport.
         * @param incoming Whether or not the connection is an incoming or outgoing connection.
         * @param adapterName The name of the adapter associated with the connection.
         * @param connectionId The connection id.
         */
        constructor(underlying?:Ice.ConnectionInfo, incoming?:boolean, adapterName?:string, connectionId?:string);
        /**
         * The information of the underyling transport or null if there's
         * no underlying transport.
         */
        underlying:Ice.ConnectionInfo;
        /**
         * Whether or not the connection is an incoming or outgoing
         * connection.
         */
        incoming:boolean;
        /**
         * The name of the adapter associated with the connection.
         */
        adapterName:string;
        /**
         * The connection id.
         */
        connectionId:string;
    }
    /**
     * This method is called by the the connection when the connection
     * is closed. If the callback needs more information about the closure,
     * it can call {@link Connection#throwException}.
     * @param con The connection that closed.
     */
    type CloseCallback = (con:Ice.Connection) => void;
    /**
     * This method is called by the the connection when a heartbeat is
     * received from the peer.
     * @param con The connection on which a heartbeat was received.
     */
    type HeartbeatCallback = (con:Ice.Connection) => void;

    /**
     * Specifies the close semantics for Active Connection Management.
     */
    class ACMClose
    {
        /**
         * Disables automatic connection closure.
         */
        static readonly CloseOff:ACMClose;
        /**
         * Gracefully closes a connection that has been idle for the configured timeout period.
         */
        static readonly CloseOnIdle:ACMClose;
        /**
         * Forcefully closes a connection that has been idle for the configured timeout period,
         * but only if the connection has pending invocations.
         */
        static readonly CloseOnInvocation:ACMClose;
        /**
         * Combines the behaviors of CloseOnIdle and CloseOnInvocation.
         */
        static readonly CloseOnInvocationAndIdle:ACMClose;
        /**
         * Forcefully closes a connection that has been idle for the configured timeout period,
         * regardless of whether the connection has pending invocations or dispatch.
         */
        static readonly CloseOnIdleForceful:ACMClose;
        static valueOf(value:number):ACMClose;
        equals(other:any):boolean;
        hashCode():number;
        toString():string;
        readonly name:string;
        readonly value:number;
    }

    /**
     * Specifies the heartbeat semantics for Active Connection Management.
     */
    class ACMHeartbeat
    {
        /**
         * Disables heartbeats.
         */
        static readonly HeartbeatOff:ACMHeartbeat;
        /**
         * Send a heartbeat at regular intervals if the connection is idle and only if there are pending dispatch.
         */
        static readonly HeartbeatOnDispatch:ACMHeartbeat;
        /**
         * Send a heartbeat at regular intervals when the connection is idle.
         */
        static readonly HeartbeatOnIdle:ACMHeartbeat;
        /**
         * Send a heartbeat at regular intervals until the connection is closed.
         */
        static readonly HeartbeatAlways:ACMHeartbeat;
        static valueOf(value:number):ACMHeartbeat;
        equals(other:any):boolean;
        hashCode():number;
        toString():string;
        readonly name:string;
        readonly value:number;
    }

    /**
     * A collection of Active Connection Management configuration settings.
     */
    class ACM
    {
        constructor(timeout?:number, close?:ACMClose, heartbeat?:ACMHeartbeat);
        clone():ACM;
        equals(rhs:any):boolean;
        hashCode():number;
        timeout:number;
        close:ACMClose;
        heartbeat:ACMHeartbeat;
        static write(outs:OutputStream, value:ACM):void;
        static read(ins:InputStream):ACM;
    }

    /**
     * Determines the behavior when manually closing a connection.
     */
    class ConnectionClose
    {
        /**
         * Close the connection immediately without sending a close connection protocol message to the peer
         * and waiting for the peer to acknowledge it.
         */
        static readonly Forcefully:ConnectionClose;
        /**
         * Close the connection by notifying the peer but do not wait for pending outgoing invocations to complete.
         * On the server side, the connection will not be closed until all incoming invocations have completed.
         */
        static readonly Gracefully:ConnectionClose;
        /**
         * Wait for all pending invocations to complete before closing the connection.
         */
        static readonly GracefullyWithWait:ConnectionClose;
        static valueOf(value:number):ConnectionClose;
        equals(other:any):boolean;
        hashCode():number;
        toString():string;
        readonly name:string;
        readonly value:number;
    }

    /**
     * The user-level interface to a connection.
     */
    interface Connection
    {
        /**
         * Manually close the connection using the specified closure mode.
         * @param mode Determines how the connection will be closed.
         * @return @returns The asynchronous result object for the invocation.
         * @see ConnectionClose
         */
        close(mode:ConnectionClose):AsyncResultBase<void>;
        /**
         * Create a special proxy that always uses this connection. This
         * can be used for callbacks from a server to a client if the
         * server cannot directly establish a connection to the client,
         * for example because of firewalls. In this case, the server
         * would create a proxy using an already established connection
         * from the client.
         * @param id The identity for which a proxy is to be created.
         * @return A proxy that matches the given identity and uses this
         * connection.
         * @see #setAdapter
         */
        createProxy(id:Identity):Ice.ObjectPrx;
        /**
         * Explicitly set an object adapter that dispatches requests that
         * are received over this connection. A client can invoke an
         * operation on a server using a proxy, and then set an object
         * adapter for the outgoing connection that is used by the proxy
         * in order to receive callbacks. This is useful if the server
         * cannot establish a connection back to the client, for example
         * because of firewalls.
         * @param adapter The object adapter that should be used by this
         * connection to dispatch requests. The object adapter must be
         * activated. When the object adapter is deactivated, it is
         * automatically removed from the connection. Attempts to use a
         * deactivated object adapter raise {@link ObjectAdapterDeactivatedException}
         * @see #createProxy
         * @see #getAdapter
         */
        setAdapter(adapter:Ice.ObjectAdapter):void;
        /**
         * Get the object adapter that dispatches requests for this
         * connection.
         * @return The object adapter that dispatches requests for the
         * connection, or null if no adapter is set.
         * @see #setAdapter
         */
        getAdapter():Ice.ObjectAdapter;
        /**
         * Get the endpoint from which the connection was created.
         * @return The endpoint from which the connection was created.
         */
        getEndpoint():Ice.Endpoint;
        /**
         * Flush any pending batch requests for this connection.
         * This means all batch requests invoked on fixed proxies
         * associated with the connection.
         * @param compress Specifies whether or not the queued batch requests
         * should be compressed before being sent over the wire.
         * @return @returns The asynchronous result object for the invocation.
         */
        flushBatchRequests(compress:CompressBatch):AsyncResultBase<void>;
        /**
         * Set a close callback on the connection. The callback is called by the
         * connection when it's closed. The callback is called from the
         * Ice thread pool associated with the connection. If the callback needs
         * more information about the closure, it can call {@link Connection#throwException}.
         * @param callback The close callback object.
         */
        setCloseCallback(callback:Ice.CloseCallback):void;
        /**
         * Set a heartbeat callback on the connection. The callback is called by the
         * connection when a heartbeat is received. The callback is called
         * from the Ice thread pool associated with the connection.
         * @param callback The heartbeat callback object.
         */
        setHeartbeatCallback(callback:Ice.HeartbeatCallback):void;
        /**
         * Send a heartbeat message.
         * @return @returns The asynchronous result object for the invocation.
         */
        heartbeat():AsyncResultBase<void>;
        /**
         * Set the active connection management parameters.
         * @param timeout The timeout value in seconds, must be &gt;= 0.
         * @param close The close condition
         * @param heartbeat The hertbeat condition
         */
        setACM(timeout:number, close:ACMClose, heartbeat:ACMHeartbeat):void;
        /**
         * Get the ACM parameters.
         * @return The ACM parameters.
         */
        getACM():ACM;
        /**
         * Return the connection type. This corresponds to the endpoint
         * type, i.e., "tcp", "udp", etc.
         * @return The type of the connection.
         */
        type():string;
        /**
         * Get the timeout for the connection.
         * @return The connection's timeout.
         */
        timeout():number;
        /**
         * Return a description of the connection as human readable text,
         * suitable for logging or error messages.
         * @return The description of the connection as human readable
         * text.
         */
        toString():string;
        /**
         * Returns the connection information.
         * @return The connection information.
         */
        getInfo():Ice.ConnectionInfo;
        /**
         * Set the connection buffer receive/send size.
         * @param rcvSize The connection receive buffer size.
         * @param sndSize The connection send buffer size.
         */
        setBufferSize(rcvSize:number, sndSize:number):void;
        /**
         * Throw an exception indicating the reason for connection closure. For example,
         * {@link CloseConnectionException} is raised if the connection was closed gracefully,
         * whereas {@link ConnectionManuallyClosedException} is raised if the connection was
         * manually closed by the application. This operation does nothing if the connection is
         * not yet closed.
         */
        throwException():void;
    }

    /**
     * Provides access to the connection details of an IP connection
     */
    class IPConnectionInfo extends ConnectionInfo
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underyling transport or null if there's no underlying transport.
         * @param incoming Whether or not the connection is an incoming or outgoing connection.
         * @param adapterName The name of the adapter associated with the connection.
         * @param connectionId The connection id.
         * @param localAddress The local address.
         * @param localPort The local port.
         * @param remoteAddress The remote address.
         * @param remotePort The remote port.
         */
        constructor(underlying?:Ice.ConnectionInfo, incoming?:boolean, adapterName?:string, connectionId?:string,
                    localAddress?:string, localPort?:number, remoteAddress?:string, remotePort?:number);
        /**
         * The local address.
         */
        localAddress:string;
        /**
         * The local port.
         */
        localPort:number;
        /**
         * The remote address.
         */
        remoteAddress:string;
        /**
         * The remote port.
         */
        remotePort:number;
    }

    /**
     * Provides access to the connection details of a TCP connection
     */
    class TCPConnectionInfo extends IPConnectionInfo
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underyling transport or null if there's no underlying transport.
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
        constructor(underlying?:Ice.ConnectionInfo, incoming?:boolean, adapterName?:string, connectionId?:string,
                    localAddress?:string, localPort?:number, remoteAddress?:string, remotePort?:number,
                    rcvSize?:number, sndSize?:number);
        /**
         * The connection buffer receive size.
         */
        rcvSize:number;
        /**
         * The connection buffer send size.
         */
        sndSize:number;
    }

    /**
     * Provides access to the connection details of a UDP connection
     */
    class UDPConnectionInfo extends IPConnectionInfo
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underyling transport or null if there's no underlying transport.
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
        constructor(underlying?:Ice.ConnectionInfo, incoming?:boolean, adapterName?:string, connectionId?:string,
                    localAddress?:string, localPort?:number, remoteAddress?:string, remotePort?:number,
                    mcastAddress?:string, mcastPort?:number, rcvSize?:number, sndSize?:number);
        /**
         * The multicast address.
         */
        mcastAddress:string;
        /**
         * The multicast port.
         */
        mcastPort:number;
        /**
         * The connection buffer receive size.
         */
        rcvSize:number;
        /**
         * The connection buffer send size.
         */
        sndSize:number;
    }

    /**
     * A collection of HTTP headers.
     */
    class HeaderDict extends Map<string, string>
    {
    }

    class HeaderDictHelper
    {
        static write(outs:OutputStream, value:HeaderDict):void;
        static read(ins:InputStream):HeaderDict;
    }

    /**
     * Provides access to the connection details of a WebSocket connection
     */
    class WSConnectionInfo extends ConnectionInfo
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underyling transport or null if there's no underlying transport.
         * @param incoming Whether or not the connection is an incoming or outgoing connection.
         * @param adapterName The name of the adapter associated with the connection.
         * @param connectionId The connection id.
         * @param headers The headers from the HTTP upgrade request.
         */
        constructor(underlying?:Ice.ConnectionInfo, incoming?:boolean, adapterName?:string,
                    connectionId?:string, headers?:HeaderDict);
        /**
         * The headers from the HTTP upgrade request.
         */
        headers:HeaderDict;
    }
}
