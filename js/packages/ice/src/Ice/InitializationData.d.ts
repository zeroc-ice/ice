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
             * Creates a deep copy of the object.
             *
             * @returns A deep copy of the object
             */
            clone(): InitializationData;

            /**
             * The properties for the communicator.
             * When not-null, this corresponds to the object returned by the Communicator::getProperties function.
             */
            properties: Properties;

            /**
             * The logger for the communicator.
             */
            logger: Logger;

            /**
             * The Slice loader, used to unmarshal Slice classes and exceptions.
             */
            sliceLoader: SliceLoader;
        }
    }
}
