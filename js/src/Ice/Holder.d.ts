// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        /**
         * A holder for a value. This interface is used to emulate `out` parameters in JavaScript.
         */
        interface Holder<T> {
            value: T;
        }
    }
}
