// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        /**
         * Represents a request received by a connection. It's the argument to {@link Object.dispatch}.
         */
        class IncomingRequest {
            /**
             * The current object for the request.
             */
            current: Current;

            /**
             * The incoming stream buffer of the request.
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
             */
            constructor(requestId: number, connection: Connection, adapter: ObjectAdapter, inputStream: InputStream);
        }
    }
}
