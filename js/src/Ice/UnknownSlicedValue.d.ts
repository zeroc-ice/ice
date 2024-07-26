//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

        class UnknownSlicedValue extends Ice.Value {
            constructor(unknownTypeId: string);

            ice_getSlicedData(): SlicedData;
            ice_id(): string;
        }
    }
}
