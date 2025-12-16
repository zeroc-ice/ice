// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Represents a request received by a connection. It's the argument to the dispatch function on Object.
         * @see {@link Object.dispatch}.
         */
        class IncomingRequest {
            /**
             * The current object of the request.
             */
            current: Current;

            /**
             * The input stream buffer of the request.
             */
            inputStream: InputStream;

            /**
             * The number of bytes in the request. These are all the bytes starting with the identity of the target.
             */
            size: number;

            /**
             * Constructs an incoming request.
             *
             * @param requestId The request ID. It's 0 for oneway requests.
             * @param connection The connection that received the request.
             * @param adapter The object adapter to set in current.
             * @param inputStream The input stream buffer over the incoming Ice protocol request message. The stream is
             * positioned at the beginning of the request header - the next data to read is the identity of the target.
             *
             * @remarks This constructor reads the request header from the input stream. When it completes, the input
             * stream is positioned at the beginning of encapsulation carried by the request.
             *
             * The Ice runtime calls this constructor when it dispatches an incoming request. You may want to call
             * it in a middleware unit test.
             */
            constructor(requestId: number, connection: Connection, adapter: ObjectAdapter, inputStream: InputStream);
        }
    }
}
