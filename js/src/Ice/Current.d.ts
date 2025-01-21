// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        /**
         * Provides information about an incoming request being dispatched.
         */
        class Current {
            /**
             * Construct a new `Current` object.
             *
             * @param adapter - The object adapter that received the request.
             * @param con - The connection that received the request.
             * @param id - The identity of the target Ice object.
             * @param facet - The facet of the target Ice object.
             * @param operation - The name of the operation being invoked.
             * @param mode - The operation mode (idempotent or not).
             * @param ctx - The request context.
             * @param requestId - The request ID. A value of `0` indicates a one-way request.
             * @param encoding - The encoding of the request payload.
             */
            constructor(
                adapter: Ice.ObjectAdapter,
                con: Ice.Connection,
                id: Identity,
                facet: string,
                operation: string,
                mode: OperationMode,
                ctx: Context,
                requestId: number,
                encoding: EncodingVersion,
            );

            /**
             * The object adapter that received the request.
             */
            adapter: Ice.ObjectAdapter;

            /**
             * The connection that received the request.
             */
            con: Ice.Connection;

            /**
             * The identity of the target Ice object.
             */
            id: Identity;

            /**
             * The facet of the target Ice object.
             */
            facet: string;

            /**
             * The name of the operation.
             */
            operation: string;

            /**
             * The operation mode (idempotent or not).
             */
            mode: OperationMode;

            /**
             * The request context.
             */
            ctx: Context;

            /**
             * The request ID. 0 means the request is a one-way request.
             */
            requestId: number;

            /**
             * The encoding of the request payload.
             */
            encoding: EncodingVersion;

            /**
             * Creates an outgoing response with reply status {@link ReplyStatus.Ok} for the request
             * being dispatched corresponding to this current object.
             *
             * @param marshal The action that marshals result into an output stream.
             * @param formatType The class format.
             * @returns A new outgoing response.
             */
            createOutgoingResponseWithResult(
                marshal: (ostr: OutputStream) => void,
                formatType?: FormatType,
            ): OutgoingResponse;

            /**
             * Creates an empty outgoing response with reply status {@link ReplyStatus.Ok} for the request
             * being dispatched corresponding to this current object.
             *
             * @returns An outgoing response with an empty payload.
             */
            createEmptyOutgoingResponse(): OutgoingResponse;

            /**
             * Creates an outgoing response that marshals an exception or the request being dispatched corresponding
             * to this current object.
             *
             * @param exception The exception to marshal into the response payload.
             * @returns A new outgoing response.
             */
            createOutgoingResponseWithException(exception: Exception): OutgoingResponse;
        }
    }
}
