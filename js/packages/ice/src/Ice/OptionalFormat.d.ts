// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Optional format for encoding optional values.
         */
        class OptionalFormat extends EnumBase {
            /**
             * Fixed 1-byte encoding.
             */
            static readonly F1: OptionalFormat;

            /**
             * Fixed 2-byte encoding.
             */
            static readonly F2: OptionalFormat;

            /**
             * Fixed 4-byte encoding.
             */
            static readonly F4: OptionalFormat;

            /**
             * Fixed 8-byte encoding.
             */
            static readonly F8: OptionalFormat;

            /**
             * "Size encoding" using either 1 or 5 bytes. Used by enums, class identifiers, etc.
             */
            static readonly Size: OptionalFormat;

            /**
             * Variable "size encoding" using either 1 or 5 bytes followed by data.
             * Used by strings, fixed size structs, and containers whose size can be computed prior to marshaling.
             */
            static readonly VSize: OptionalFormat;

            /**
             * Fixed "size encoding" using 4 bytes followed by data.
             * Used by variable-size structs, and containers whose sizes can't be computed prior to unmarshaling.
             */
            static readonly FSize: OptionalFormat;

            /**
             * Class instance. No longer supported.
             */
            static readonly Class: OptionalFormat;

            /**
             * Returns the enumerator corresponding to the specified numeric value.
             *
             * @param value The enumerator value.
             * @returns The matching enumerator, or `undefined` if no enumerator corresponds to the given value.
             */
            static valueOf(value: number): OptionalFormat | undefined;
        }
    }
}
