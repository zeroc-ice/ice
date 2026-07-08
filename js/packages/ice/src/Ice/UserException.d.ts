// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Base class for all Ice user exceptions.
         */
        abstract class UserException extends Exception {
            constructor();
        }
    }
}
