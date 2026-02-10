// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         *  This enumeration describes the possible formats for classes.
         */
        class FormatType extends Ice.EnumBase {
            /**
             * Favors compactness, but does not support slicing-off unknown slices during unmarshaling.
             */
            static readonly CompactFormat: FormatType;

            /**
             * Allows slicing-off unknown slices during unmarshaling, at the cost of some extra space in the marshaled
             * data.
             */
            static readonly SlicedFormat: FormatType;

            /**
             * Returns the enumerator corresponding to the specified numeric value.
             *
             * @param value - The numeric value of the enumerator.
             * @returns The matching enumerator, or `undefined` if no enumerator corresponds to the given value.
             */
            static valueOf(value: number): FormatType | undefined;
        }
    }
}
