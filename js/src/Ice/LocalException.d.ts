// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Base class for all Ice exceptions not defined in Slice.
         */
        abstract class LocalException extends Exception {}
    }
}
