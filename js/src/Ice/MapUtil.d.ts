// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        class MapUtil {
            /**
             * Compares two maps for equality.
             *
             * @param lhs The first map to compare.
             * @param rhs The second map to compare.
             * @returns `true` if the maps are equal, `false` otherwise.
             */
            static equals<K, V>(lhs: Map<K, V>, rhs: Map<K, V>): boolean;
        }
    }
}
