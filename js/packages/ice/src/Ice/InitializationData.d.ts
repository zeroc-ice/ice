// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Represents a set of options that you can specify when initializing a communicator.
         */
        class InitializationData {
            /**
             * Construct a new InitializationData instance.
             */
            constructor();

            /**
             * Creates a shallow copy of this object.
             *
             * @returns A shallow copy of this object
             */
            clone(): InitializationData;

            /**
             * The properties for the communicator.
             * When not-null, this corresponds to the object returned by {@link Communicator#getProperties}.
             */
            properties: Properties | null;

            /**
             * The logger for the communicator.
             */
            logger: Logger | null;

            /**
             * The Slice loader, used to unmarshal Slice classes and exceptions.
             */
            sliceLoader: SliceLoader | null;
        }
    }
}
