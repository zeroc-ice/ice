//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        /**
         * Base class for all Ice user exceptions.
         */
        abstract class UserException extends Exception {
            /**
             * Obtains the Slice type ID of this exception.
             * @return The fully-scoped type ID.
             */
            static ice_staticId(): string;
        }
    }
}
