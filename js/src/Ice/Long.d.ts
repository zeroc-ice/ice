// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Represents a 64-bit signed integer.
         */
        class Long {
            /**
             * Constructs a new `Long` object from two 32-bit words.
             *
             * If only one argument is provide we assume it is a JavaScript Number, and we convert it to two 32 bit
             * words to fit in the Ice.Long internal representation.
             *
             * If two arguments are provided we assume these are the high and low words respectively.
             *
             * @param high The high 32 bits.
             * @param low The low 32 bits.
             */
            constructor(high?: number, low?: number);

            /**
             * Returns the hash code of the current value.
             *
             * @returns The hash code of the current value.
             */
            hashCode(): number;

            /**
             * Determines whether the specified object is equal to this object.
             *
             * @param other The object to compare with.
             * @returns `true` if the specified object is equal to the this object, `false` otherwise.
             */
            equals(other: Long): boolean;

            /**
             * Returns a string representation of the current value.
             *
             * @returns A string representation of the current value.
             */
            toString(): string;

            /**
             * Converts the current value to a JavaScript number. If the value is too large to fit in a JavaScript number
             * this function will return `Infinity` or `-Infinity`.
             *
             * @returns The current value as a JavaScript number.
             */
            toNumber(): number;

            low: number;
            high: number;
        }
    }
}
