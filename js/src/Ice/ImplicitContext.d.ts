//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        /**
         * An interface to associate implicit contexts with communicators.
         *
         * When you make a remote invocation without an explicit context parameter, Ice uses the per-proxy context
         * (if any) combined with the `ImplicitContext` associated with the communicator.
         *
         * Ice provides two implementations of `ImplicitContext`. The implementation used depends on the value of
         * the `Ice.ImplicitContext` property.
         *
         * - None (default): No implicit context at all.
         * - Shared: The implementation maintains a single context shared by all invocations.
         *
         * The `ImplicitContext` interface also provides a number of operations to create, update or retrieve an entry
         * in the underlying context without first retrieving a copy of the entire context.
         */
        interface ImplicitContext {
            /**
             * Get a copy of the underlying context.
             *
             * @returns A copy of the underlying context.
             */
            getContext(): Context;

            /**
             * Set the underlying context.
             *
             * @param newContext The new context.
             */
            setContext(newContext: Context): void;

            /**
             * Check if this key has an associated value in the underlying context.
             *
             * @param key The key.
             * @returns `true` if the key has an associated value, `false` otherwise.
             */
            containsKey(key: string): boolean;

            /**
             * Get the value associated with the given key in the underlying context.
             *
             * Returns an empty string if no value is associated with the key. {@link ImplicitContext#containsKey}
             * allows you to distinguish between an empty-string value and no value at all.
             *
             * @param key The key.
             * @returns The value associated with the key.
             */
            get(key: string): string;

            /**
             * Create or update a key/value entry in the underlying context
             * .
             * @param key The key.
             * @param value The value.
             * @returns The previous value associated with the key, if any.
             */
            put(key: string, value: string): string;

            /**
             * Remove the entry for the given key in the underlying context.
             *
             * @param key The key.
             * @returns The value associated with the key, if any.
             */
            remove(key: string): string;
        }
    }
}
