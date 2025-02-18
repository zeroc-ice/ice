// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Base class for all Ice exceptions.
         */
        abstract class Exception extends Error {
            /**
             * Returns the type id of this exception.
             *
             * @returns The type id of this exception.
             **/
            ice_id(): string;

            /**
             * Returns a string representation of this exception.
             *
             * @returns A string representation of this exception.
             **/
            toString(): string;
        }
    }
}
