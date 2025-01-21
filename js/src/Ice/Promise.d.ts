// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        /**
         * The `Promise` class is a subclass of the global `Promise` class that adds the ability to resolve or reject the
         * promise from outside of the executor function.
         */
        class Promise<T> extends globalThis.Promise<T> {
            /**
             * Constructs a new promise.
             *
             *  @param executor - The executor function that is called immediately when the promise is constructed. It
             *                    is passed two functions, `resolve` and `reject`, that can be called to resolve or
             *                    reject the promise respectively.
             */
            constructor(
                executor?: (resolve: (value: T | PromiseLike<T>) => void, reject: (reason: any) => void) => void,
            );

            /**
             * Resolves the promise with the specified value.
             *
             * @param value - The value to resolve the promise with.
             */
            resolve<T>(value?: T | PromiseLike<T>): void;

            /**
             * Rejects the promise with the specified reason.
             *
             * @param reason - The reason for rejecting the promise.
             */
            reject(reason: any): void;

            /**
             * Returns a promise that is resolved after a specified delay.
             *
             * @param ms - The number of milliseconds to delay.
             * @returns A promise that is resolved after the specified delay.
             */
            static delay(ms: number): Promise<void>;

            /**
             * Returns a promise that is resolved after a specified delay with a specified value.
             *
             * @param ms - The number of milliseconds to delay.
             * @param value - The value to resolve the promise with.
             * @returns A promise that is resolved after the specified delay with the specified value.
             */
            static delay<T>(ms: number, value: T): Promise<T>;
        }
    }
}
