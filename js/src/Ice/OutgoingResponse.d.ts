// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Represents the response to an incoming request. It's returned by {@link Object#dispatch}.
         */
        class OutgoingResponse {
            /**
             * Constructs an outgoing response.
             *
             * @param outputStream The output stream that holds the response.
             * @param replyStatus The reply status.
             * @param exceptionId The ID of the exception, when the response carries an exception.
             * @param exceptionDetails The full details of the exception, when the response carries an exception.
             */
            constructor(
                outputStream: OutputStream,
                replyStatus: ReplyStatus,
                exceptionId?: string,
                exceptionDetails?: string,
            );

            /**
             * The reply status of the response.
             */
            replyStatus: ReplyStatus;

            /**
             * The output stream buffer of the response. This output stream should not be written to after construction.
             */
            outputStream: OutputStream;

            /**
             * The exception ID of the response. It's null when replyStatus is {@link ReplyStatus.Ok}.
             */
            exceptionId: string;

            /**
             * The exception details marshaled into the response. It's null when replyStatus is {@link ReplyStatus.Ok}.
             */
            exceptionDetails: string;

            /**
             * The number of bytes in the response's payload.
             */
            size: number;
        }
    }
}
