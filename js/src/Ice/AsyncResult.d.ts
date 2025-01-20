// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        /**
         * The `AsyncResult` class encapsulates the state of an asynchronous invocation. It extends the {@link Promise}
         * class. It provides additional methods to support canceling the operation.
         */
        abstract class AsyncResult<T> extends Promise<T> {
            /**
             * Cancels the asynchronous request.
             */
            cancel(): void;

            /**
             * Whether the asynchronous request has completed.
             *
             * @returns `true` if the asynchronous request has completed, `false` otherwise.
             */
            isCompleted(): boolean;

            /**
             * Whether the asynchronous request has been sent.
             *
             * @returns `true` if the asynchronous request has been sent, `false` otherwise.
             */
            isSent(): boolean;

            /**
             * Throws the exception if the asynchronous request was rejected with one; otherwise, does nothing.
             */
            throwLocalException(): void;

            /**
             * Whether the asynchronous request has been sent synchronously.
             *
             * @returns `true` if the asynchronous request has been sent synchronously, `false` otherwise.
             */
            sentSynchronously(): boolean;

            /**
             * The communicator associated with the asynchronous operation.
             */
            readonly communicator: Communicator;

            /**
             * The object proxy associated with the invocation.
             */
            readonly proxy: ObjectPrx;

            /**
             * The operation name.
             */
            readonly operation: string;
        }
    }
}
