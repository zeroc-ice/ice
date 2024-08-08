//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        /**
         * Base class for enumerated types.
         */
        class EnumBase {
            /**
             * Constructs a new enumerator.
             *
             * @param name The name of the enumerator.
             * @param value The value of the enumerator.
             */
            constructor(name: string, value: number);

            /**
             * Determines whether the specified object is equal to the current value.
             *
             * @param other The object to compare with the current value.
             * @returns `true` if the specified object is equal to the current value, `false` otherwise.
             */
            equals(other: any): boolean;

            /**
             * Returns the hash code of the current value.
             *
             * @returns The hash code of the current value.
             */
            hashCode(): number;

            /**
             * Returns a string representation of the current value.
             *
             * @returns A string representation of the current value.
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
