// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * An application-provided class that an object adapter uses to locate servants.
         *
         * @remarks Servant locators are provided for backward compatibility with earlier versions of Ice. You should
         * consider using a default servant instead (see {@link ObjectAdapter#addDefaultServant}). For more advanced
         * use cases, you can create a middleware (see {@link ObjectAdapter#use}).
         * @see {@link ObjectAdapter#addServantLocator}
         */
        interface ServantLocator {
            /**
             * Asks this servant locator to find and return a servant.
             *
             * @remarks The caller (the object adapter) does not insert the returned servant into its Active Servant
             * Map.
             *
             * The implementation can throw any exception, including UserException. The Ice runtime marshals this
             * exception in the response.
             *
             * @param current - Information about the incoming request being dispatched.
             * @returns An array where:
             *  - [0]: The located servant, or `null` if no suitable servant was found.
             *  - [1]: A "cookie" that will be passed to {@link finished}.
             * @throws {@link UserException} - The implementation can raise a `UserException`, and the runtime will
             * marshal it as the result of the invocation.
             *
             * @see {@link ObjectAdapter}
             * @see {@link finished}
             */
            locate(current: Current): [Ice.Object | null, object | null];

            /**
             * Notifies this servant locator that the dispatch on the servant returned by {@link locate} is complete.
             * The object adapter calls this function only when {@link locate} returns a non-null servant.
             *
             * @remarks The implementation can throw any exception, including {@link UserException}. The Ice runtime
             * marshals this  exception in the response. If both the dispatch and `finished` throw an exception, the
             * exception thrown by `finished` prevails and is marshaled back to the client.
             *
             * @param current - Information about the incoming request being dispatched.
             * @param servant - The servant that was returned by `locate`.
             * @param cookie - The cookie that was returned by `locate`.
             * @throws {@link UserException} - The implementation can raise a `UserException`, and the runtime will marshal it as the
             * result of the invocation.
             *
             * @see {@link ObjectAdapter}
             * @see {@link locate}
             */
            finished(current: Current, servant: Ice.Object, cookie: object | null): void;

            /**
             * Notifies this servant locator that the object adapter in which it's installed is being deactivated.
             *
             * @param category - The category with which this servant locator was registered.
             *
             * @see {@link ObjectAdapter#destroy}
             */
            deactivate(category: string): void;
        }
    }
}
