// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
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
