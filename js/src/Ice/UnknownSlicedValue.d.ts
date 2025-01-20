// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        /**
         * Encapsulates the details of a slice with an unknown type.
         */
        class SliceInfo {
            /**
             * The Slice type ID for this slice.
             */
            typeId: string;

            /**
             * The Slice compact type ID for this slice.
             */
            compactId: number;

            /**
             * The encoded bytes for this slice, including the leading size integer.
             */
            bytes: Uint8Array;

            /**
             * The class instances referenced by this slice.
             */
            instances: Value[];

            /**
             * Whether or not the slice contains optional members.
             */
            hasOptionalMembers: boolean;

            /**
             * Whether or not this is the last slice.
             */
            isLastSlice: boolean;
        }

        /**
         * Holds class slices that cannot be unmarshaled because their types are not known locally.
         */
        class SlicedData {
            /**
             * The details of each slice, in order of most-derived to least-derived.
             */
            slices: SliceInfo[];
        }

        /**
         * Unknown sliced value holds an instance of an unknown Slice class type.
         */
        class UnknownSlicedValue extends Ice.Value {
            /**
             * Constructs an unknown sliced value.
             *
             * @param unknownTypeId The Slice type ID of the unknown class.
             */
            constructor(unknownTypeId: string);

            /**
             * Returns the Slice type ID associated with this object.
             *
             * @returns The Slice type ID.
             */
            ice_id(): string;
        }
    }
}
