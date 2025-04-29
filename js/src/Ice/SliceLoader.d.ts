// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Creates class and exception instances from Slice type IDs.
         */
        interface SliceLoader {
            /**
             * Creates an instance of a class mapped from a Slice class or exception based on a Slice type ID.
             *
             * @param typeId The Slice type ID or compact type ID.
             * @return A new instance of the class or exception identified by `typeId`, or `null` if the implementation
             *         cannot find the corresponding class.
             * @throws MarshalException If the corresponding class was found but its instantiation failed.
             */
            newInstance(typeId: string): Ice.Value | Ice.UserException | null;
        }

        /**
         * Implements SliceLoader by combining several Slice loaders.
         */
        class CompositeSliceLoader implements SliceLoader {
            /**
             * Constructs a new `CompositeSliceLoader` object.
             * @param sliceLoaders The initial list of Slice loaders; can be empty.
             */
            constructor(sliceLoaders: SliceLoader[]);

            /**
             * Adds a Slice loader to this composite Slice loader.
             * @param sliceLoader The Slice loader to add.
             */
            add(sliceLoader: SliceLoader): void;

            newInstance(typeId: string): Ice.Value | Ice.UserException | null;
        }
    }
}
