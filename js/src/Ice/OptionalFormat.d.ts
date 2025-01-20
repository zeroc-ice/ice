// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        /**
         * Optional format for encoding optional values.
         */
        class OptionalFormat extends EnumBase {
            /**
             * A fixed size numeric encoded on 1 byte.
             */
            static readonly F1: OptionalFormat;

            /**
             * A fixed size numeric encoded on 2 byte.
             */
            static readonly F2: OptionalFormat;

            /**
             * A fixed size numeric encoded on 4 byte.
             */
            static readonly F4: OptionalFormat;

            /**
             * A fixed size numeric encoded on 8 byte.
             */
            static readonly F8: OptionalFormat;

            /**
             * A variable-length size encoded on 1 or 5 bytes.
             */
            static readonly Size: OptionalFormat;

            /**
             * A variable-length size followed by size bytes.
             */
            static readonly VSize: OptionalFormat;

            /**
             * A fixed length size (encoded on 4 bytes) followed by size bytes.
             */
            static readonly FSize: OptionalFormat;

            /**
             * Represents a class, but is no longer encoded or decoded.
             */
            static readonly Class: OptionalFormat;

            /**
             * Returns the enumerator for the given value.
             *
             * @param value The enumerator value.
             * @returns The enumerator for the given value.
             */
            static valueOf(value: number): OptionalFormat;
        }
    }
}
