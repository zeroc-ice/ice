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
    }
}
