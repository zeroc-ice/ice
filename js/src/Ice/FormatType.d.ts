// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        /**
         *  This enumeration describes the possible formats for classes.
         */
        class FormatType extends Ice.EnumBase {
            /**
             * The Compact format assumes the sender and receiver have the same Slice definitions for classes. If an
             * application receives a derived class it does not know, it is not capable of decoding it into a known
             * base class because there is not enough information in the encoded payload. The compact format is more
             * space-efficient on the wire.
             */
            static readonly CompactFormat: FormatType;

            /**
             * The Sliced format allows the receiver to slice off unknown slices. If an application receives a derived
             * class it does not know, it can create a base class while preserving the unknown derived slices.
             */
            static readonly SlicedFormat: FormatType;

            /**
             * Returns the enumerator for the given value.
             *
             * @param value - The numeric value of the enumerator.
             * @returns The enumerator corresponding to the given value.
             */
            static valueOf(value: number): FormatType;
        }
    }
}
