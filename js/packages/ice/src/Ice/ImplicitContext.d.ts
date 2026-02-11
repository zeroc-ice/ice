// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Represents the request context associated with a communicator.
         * When you make a remote invocation without an explicit request context parameter, Ice uses the per-proxy
         * request context (if any) combined with the `ImplicitContext` associated with your communicator. The property
         * `Ice.ImplicitContext` controls if your communicator has an associated implicit context.
         */
        interface ImplicitContext {
            /**
             * Gets a copy of the request context maintained by this object.
             *
             * @returns A copy of the request context.
             */
            getContext(): Context;

            /**
             * Sets the request context.
             *
             * @param newContext - The new request context.
             */
            setContext(newContext: Context): void;

            /**
             * Checks if the specified key has an associated value in the request context.
             *
             * @param key - The key.
             * @returns `true` if the key has an associated value, `false` otherwise.
             */
            containsKey(key: string): boolean;

            /**
             * Gets the value associated with the specified key in the request context.
             *
             * @param key - The key.
             * @returns The value associated with the key, or the empty string if no value is associated with the key.
             */
            get(key: string): string;

            /**
             * Creates or updates a key/value entry in the request context.
             *
             * @param key - The key.
             * @param value - The value.
             * @returns The previous value associated with the key, if any.
             */
            put(key: string, value: string): string;

            /**
             * Removes the entry for the specified key in the request context.
             *
             * @param key - The key.
             * @returns The value associated with the key, if any.
             */
            remove(key: string): string;
        }
    }
}
