// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Abstract base class for all Ice exceptions. It has only two derived classes: LocalException and UserException.
         */
        abstract class Exception extends Error {
            /**
             * Returns the type ID of this exception. This corresponds to the Slice type ID for Slice-defined exceptions,
             * and to a similar fully scoped name for other exceptions. For example
             * "::Ice::CommunicatorDestroyedException".
             *
             * @returns The type id of this exception.
             */
            ice_id(): string;

            /**
             * Returns a string representation of this exception.
             *
             * @returns A string representation of this exception.
             */
            toString(): string;
        }
    }
}
