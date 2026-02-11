// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Options for constructing an Error with a cause.
         *
         * @see {@link https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Error/Error#rethrowing_an_error_with_a_cause}
         */
        interface ErrorOptions {
            cause?: unknown;
        }

        //
        // Dispatch exceptions
        //

        /**
         * The exception that is thrown when a dispatch failed. This is the base class for local exceptions that can be
         * marshaled and transmitted "over the wire".
         * You can throw this exception in the implementation of an operation, or in a middleware. The Ice runtime then
         * logically rethrows this exception to the client.
         */
        class DispatchException extends LocalException {
            constructor(replyStatus: ReplyStatus, message?: string);
            readonly replyStatus: ReplyStatus;
        }

        /**
         * The base class for the 3 NotExist exceptions.
         */
        class RequestFailedException extends DispatchException {
            readonly id: Identity;
            readonly facet: string;
            readonly operation: string;
        }

        /**
         * The exception that is thrown when a dispatch cannot find a servant for the identity carried by the request.
         */
        class ObjectNotExistException extends RequestFailedException {
            constructor(id?: Identity, facet?: string, operation?: string);
        }

        /**
         * The exception that is thrown when a dispatch cannot find a servant for the identity + facet carried by the
         * request.
         */
        class FacetNotExistException extends RequestFailedException {
            constructor(id?: Identity, facet?: string, operation?: string);
        }

        /**
         * The exception that is thrown when a dispatch cannot find the operation carried by the request on the target
         * servant. This is typically due to a mismatch in the Slice definitions, such as the client using Slice
         * definitions newer than the server's.
         */
        class OperationNotExistException extends RequestFailedException {
            constructor(id?: Identity, facet?: string, operation?: string);
        }

        /**
         * The exception that is thrown when a dispatch failed with an exception that is not a {@link LocalException}
         * or a {@link UserException}.
         */
        class UnknownException extends DispatchException {
            /**
             * Constructs an unknown exception.
             * @param message The exception message.
             */
            constructor(message: string);
            readonly unknown: string;
        }

        /**
         * The exception that is thrown when a dispatch failed with a {@link LocalException} that is not a
         * {@link DispatchException}.
         */
        class UnknownLocalException extends UnknownException {
            /**
             * Constructs an unknown local exception.
             * @param message The exception message.
             */
            constructor(message: string);
        }

        /**
         * The exception that is thrown when a client receives a {@link UserException} that was not declared in the
         * operation's exception specification.
         */
        class UnknownUserException extends UnknownException {
            /**
             * Constructs an unknown user exception.
             * @param message The exception message.
             */
            constructor(message: string);
        }

        //
        // Protocol exceptions
        //

        /**
         * The base class for exceptions related to the Ice protocol.
         */
        class ProtocolException extends LocalException {}

        /**
         * The exception that is thrown when the connection has been gracefully shut down by the server. The request
         * that returned this exception has not been executed by the server. In most cases you will not get this
         * exception, because the client will automatically retry the invocation. However, if upon retry the server
         * shuts down the connection again, and the retry limit has been reached, then this exception is propagated
         * to the application code.
         */
        class CloseConnectionException extends ProtocolException {
            constructor();
        }

        /**
         *  The exception that is thrown when an error occurs during marshaling or unmarshaling.
         */
        class MarshalException extends ProtocolException {}

        //
        // Timeout exceptions
        //

        /**
         * The exception that is thrown when a timeout occurs. This is the base class for all timeout exceptions.
         */
        class TimeoutException extends LocalException {}

        /**
         * The exception that is thrown when a graceful connection closure times out.
         */
        class CloseTimeoutException extends TimeoutException {
            constructor();
        }

        /**
         * The exception that is thrown when a connection establishment times out.
         */
        class ConnectTimeoutException extends TimeoutException {
            constructor();
        }

        /**
         * This exception indicates that an invocation failed because it timed out.
         */
        class InvocationTimeoutException extends TimeoutException {
            constructor();
        }

        //
        // Syscall exceptions
        //

        /**
         * The exception that is thrown to report the failure of a system call.
         */
        class SyscallException extends LocalException {}

        //
        // Socket exceptions
        //

        /**
         * The exception that is thrown to report a socket error.
         */
        class SocketException extends SyscallException {}

        /**
         * The exception that is thrown when a connection establishment fails.
         */
        class ConnectFailedException extends SocketException {
            /**
             * Constructs a ConnectFailedException.
             *
             * @param peerAddress The address of the remote peer, if available.
             * @param options The error options.
             */
            constructor(peerAddress?: string, options?: ErrorOptions);
        }

        /**
         * The exception that is thrown when an established connection is lost.
         */
        class ConnectionLostException extends SocketException {
            /**
             * Constructs a ConnectionLostException.
             *
             * @param peerAddress The address of the remote peer, if available.
             * @param options The error options.
             */
            constructor(peerAddress?: string, options?: ErrorOptions);
        }

        /**
         * The exception that is thrown when the server host actively refuses a connection.
         */
        class ConnectionRefusedException extends ConnectFailedException {
            /**
             * Constructs a ConnectionRefusedException.
             *
             * @param serverAddress The address of the remote server, if available.
             * @param options The error options.
             */
            constructor(serverAddress?: string, options?: ErrorOptions);
        }

        //
        // Other leaf local exceptions in alphabetical order.
        //

        /**
         * The exception that is thrown when you attempt to register an object more than once with the Ice runtime.
         */
        class AlreadyRegisteredException extends LocalException {
            constructor(kindOfObject: string, id: string);
            readonly kindOfObject: string;
            readonly id: string;
        }

        /**
         * The exception that is thrown when an operation fails because the communicator has been destroyed.
         * @see {@link Communicator#destroy()}
         */
        class CommunicatorDestroyedException extends LocalException {}

        /**
         * The exception that is thrown when an operation fails because the connection has been aborted.
         */
        class ConnectionAbortedException extends LocalException {
            constructor(message: string, closedByApplication: boolean);
            readonly closedByApplication: boolean;
        }

        /**
         * The exception that is thrown when an operation fails because the connection has been closed gracefully.
         */
        class ConnectionClosedException extends LocalException {
            constructor(message: string, closedByApplication: boolean);
            readonly closedByApplication: boolean;
        }

        /**
         * The exception that is thrown when attempting to use an unsupported feature.
         */
        class FeatureNotSupportedException extends LocalException {
            constructor(message: string);
        }

        /**
         * The exception that is thrown when attempting to change a connection-related property on a fixed proxy.
         */
        class FixedProxyException extends LocalException {}

        /**
         * The exception that is thrown when communicator initialization fails.
         */
        class InitializationException extends LocalException {}

        /**
         * This exception indicates that an asynchronous invocation failed because it was canceled explicitly by the user.
         */
        class InvocationCanceledException extends LocalException {
            constructor();
        }

        /**
         * The exception that is thrown when the Ice runtime cannot find a suitable endpoint to connect to.
         */
        class NoEndpointException extends LocalException {
            constructor(messageOrProxy: string | ObjectPrx);
        }

        /**
         * The exception that is thrown when attempting to find or deregister something that is not registered with
         * Ice.
         */
        class NotRegisteredException extends LocalException {
            /**
             * Constructs a NotRegisteredException with the object kind and ID.
             *
             * @param kindOfObject The kind of object that is not registered.
             * @param id           The ID (or name) of the object that is not registered.
             */
            constructor(kindOfObject?: string, id?: string);

            /**
             * The kind of object that is not registered.
             */
            readonly kindOfObject: string;

            /**
             * The ID (or name) of the object that is not registered.
             */
            readonly id: string;
        }

        /**
         * The exception that is thrown when attempting to use an ObjectAdapter that has been destroyed.
         */
        class ObjectAdapterDestroyedException extends LocalException {
            constructor(name: string);
        }

        /**
         * The exception that is thrown when the parsing of a string fails.
         */
        class ParseException extends LocalException {}

        /**
         * The exception that is thrown when attempting to invoke an operation with `ice_oneway`, `ice_batchOneway`,
         * and the operation has a return value, an out parameter, or an exception specification.
         */
        class TwowayOnlyException extends LocalException {
            constructor(operation: string);
        }

        /**
         * The exception that is thrown when a property cannot be set or retrieved. For example, this exception is
         * thrown when attempting to set an unknown Ice property.
         */
        class PropertyException extends LocalException {
            constructor(message: string);
        }
    }
}
