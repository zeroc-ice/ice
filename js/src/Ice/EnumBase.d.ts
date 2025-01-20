// Copyright (c) ZeroC, Inc.

declare module "ice" {
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
             * Determines whether the specified object is equal to the current enumerator.
             *
             * @param other - The object to compare with the current enumerator.
             * @returns `true` if the specified object is equal to the current enumerator, `false` otherwise.
             */
            equals(other: any): boolean;

            /**
             * Returns the hash code of the current enumerator.
             *
             * @returns The hash code of the current enumerator.
             */
            hashCode(): number;

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
