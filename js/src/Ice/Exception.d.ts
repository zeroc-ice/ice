//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        /**
         * Base class for all Ice exceptions.
         */
        abstract class Exception extends Error {
            /**
             * Returns the type id of this exception.
             *
             * @return The type id of this exception.
             **/
            ice_id(): string;

            /**
             * Returns a string representation of this exception.
             *
             * @return A string representation of this exception.
             **/
            toString(): string;

            ice_cause: string | Error;
        }
    }
}
