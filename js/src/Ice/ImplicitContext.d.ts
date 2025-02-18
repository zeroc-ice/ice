// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Interface to associate implicit contexts with communicators.
         *
         * When you make a remote invocation without an explicit context parameter, Ice uses the per-proxy context
         * (if any) combined with the `ImplicitContext` associated with the communicator.
         *
         * Ice provides two implementations of `ImplicitContext`, depending on the value of the `Ice.ImplicitContext` property:
         *
         * - None (default): No implicit context is used.
         * - Shared: A single context is shared by all invocations.
         *
         * The `ImplicitContext` interface provides several operations to create, update, or retrieve an entry
         * in the underlying context without retrieving a copy of the entire context.
         */
        interface ImplicitContext {
            /**
             * Retrieves a copy of the underlying context.
             *
             * @returns A copy of the underlying context.
             */
            getContext(): Context;

            /**
             * Sets the underlying context.
             *
             * @param newContext - The new context.
             */
            setContext(newContext: Context): void;

            /**
             * Checks if this key has an associated value in the underlying context.
             *
             * @param key - The key.
             * @returns `true` if the key has an associated value, `false` otherwise.
             */
            containsKey(key: string): boolean;

            /**
             * Retrieves the value associated with the given key in the underlying context.
             *
             * Returns an empty string if no value is associated with the key. Use {@link ImplicitContext#containsKey}
             * to distinguish between an empty-string value and no value at all.
             *
             * @param key - The key.
             * @returns The value associated with the key, or an empty string if no value is associated with the key.
             */
            get(key: string): string;

            /**
             * Creates or updates a key/value entry in the underlying context.
             *
             * @param key - The key.
             * @param value - The value.
             * @returns The previous value associated with the key, if any.
             */
            put(key: string, value: string): string;

            /**
             * Removes the entry for the given key in the underlying context.
             *
             * @param key - The key.
             * @returns The value that was associated with the key, if any.
             */
            remove(key: string): string;
        }
    }
}
