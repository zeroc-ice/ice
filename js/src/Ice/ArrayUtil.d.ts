// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        /**
         * Utility functions for arrays.
         */
        class ArrayUtil {
            /**
             * Creates a shallow copy of the array.
             * @param arr The array to clone.
             * @returns A shallow copy of the array.
             */
            static clone<T>(arr: T[]): T[];

            /**
             * Compares two arrays for equality.
             * @param lhs The first array to compare.
             * @param rhs The second array to compare.
             * @param valuesEqual An optional function that compares two values for equality. If not provided, the
             * comparison is done using the elements default equal comparator.
             * @returns True if the arrays are equal, false otherwise.
             */
            static equals(
                lhs: any[] | Uint8Array,
                rhs: any[] | Uint8Array,
                valuesEqual?: (v1: any, v2: any) => boolean,
            ): boolean;
        }
    }
}
