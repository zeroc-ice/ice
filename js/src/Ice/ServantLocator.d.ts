// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        /**
         * A servant locator is called by an object adapter to locate a servant that is not found in its active servant map.
         * @see {@link ObjectAdapter}
         * @see {@link ObjectAdapter#addServantLocator}
         * @see {@link ObjectAdapter#findServantLocator}
         */
        interface ServantLocator {
            /**
             * Called before a request is dispatched if a servant cannot be found in the object adapter's active servant map.
             * Note that the object adapter does not automatically insert the returned servant into its active servant map.
             * This must be done by the servant locator implementation, if desired. The `locate` method can throw any
             * user exception. If it does, that exception is marshaled back to the client. If the Slice definition for the
             * corresponding operation includes that user exception, the client receives it; otherwise, the client receives
             * an {@link UnknownUserException}.
             *
             * If `locate` throws any exception, the Ice runtime does *not* call `finished`.
             *
             * @param current - Information about the current operation for which a servant is required.
             * @param cookie - A "cookie" that will be passed to `finished`.
             * @returns The located servant, or `null` if no suitable servant has been found.
             * @throws {@link UserException} - The implementation can raise a `UserException`, and the runtime will marshal it as the
             * result of the invocation.
             *
             * @see {@link ObjectAdapter}
             * @see {@link finished}
             */
            locate(current: Current, cookie: Holder<Object>): Ice.Object | null;

            /**
             * Called by the object adapter after a request has been processed. This operation is only invoked if
             * `locate` was called before the request and returned a non-null servant. The `finished` method can be used
             * for cleanup purposes after a request.
             *
             * The `finished` method can throw any user exception. If it does, that exception is marshaled back to the client.
             * If the Slice definition for the corresponding operation includes that user exception, the client receives it;
             * otherwise, the client receives an {@link UnknownUserException}.
             * If both the operation and `finished` throw exceptions, the exception thrown by `finished` is marshaled back to the client.
             *
             * @param current - Information about the current operation call for which a servant was located by `locate`.
             * @param servant - The servant that was returned by `locate`.
             * @param cookie - The cookie that was returned by `locate`.
             * @throws {@link UserException} - The implementation can raise a `UserException`, and the runtime will marshal it as the
             * result of the invocation.
             *
             * @see {@link ObjectAdapter}
             * @see {@link locate}
             */
            finished(current: Current, servant: Ice.Object, cookie: Object): void;

            /**
             * Called when the object adapter in which this servant locator is installed is destroyed.
             *
             * @param category - The category for which the servant locator is being deactivated.
             *
             * @see {@link ObjectAdapter#destroy}
             */
            deactivate(category: string): void;
        }
    }
}
