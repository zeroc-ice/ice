// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Base class for enumerated types.
         */
        class EnumBase {
            /**
             * Constructs a new enumerator.
             *
             * @param name - The name of the enumerator.
             * @param value - The numeric value of the enumerator.
             */
            constructor(name: string, value: number);

            /**
             * Returns a string representation of the current enumerator.
             *
             * @returns A string representation of the current enumerator.
             */
            toString(): string;

            /**
             * The name of the enumerator.
             */
            readonly name: string;

            /**
             * The value of the enumerator.
             */
            readonly value: number;
        }
    }
}
