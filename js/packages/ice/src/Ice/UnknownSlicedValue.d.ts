// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Encapsulates the details of a slice with an unknown type.
         */
        class SliceInfo {
            /**
             * The Slice type ID for this slice. It's empty when the compact ID is set (compactId !== -1).
             */
            typeId: string;

            /**
             * The Slice compact type ID for this slice.
             */
            compactId: number;

            /**
             * The encoded bytes for this slice.
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
             * @param unknownTypeId The Slice type ID of the unknown class, or the string form of the compact type
             * ID (for example, "1") when the most-derived slice was marshaled with a compact type ID.
             */
            constructor(unknownTypeId: string);

            /**
             * Returns the Slice type ID associated with this object.
             *
             * @returns The type ID supplied to the constructor. It's the string form of the compact type ID (for
             * example, "1") when the most-derived slice was marshaled with a compact type ID.
             */
            ice_id(): string;
        }
    }
}
