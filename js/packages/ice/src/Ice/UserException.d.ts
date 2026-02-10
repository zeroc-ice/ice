// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Base class for all Ice user exceptions.
         */
        abstract class UserException extends Exception {
            constructor();

            /**
             * Obtains the Slice type ID of this exception.
             * @returns The fully-scoped type ID.
             */
            static ice_staticId(): string;
        }
    }
}
