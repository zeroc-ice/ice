// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Determines how the Ice runtime sorts proxy endpoints when establishing a connection.
         */
        class EndpointSelectionType extends Ice.EnumBase {
            /**
             * The Ice runtime shuffles the endpoints in a random order.
             */
            static readonly Random: EndpointSelectionType;

            /**
             * The Ice runtime uses the endpoints in the order they appear in the proxy.
             */
            static readonly Ordered: EndpointSelectionType;

            /**
             * Returns the enumerator corresponding to the specified numeric value.
             *
             * @param value - The numeric value of the enumerator.
             * @returns The matching enumerator, or `undefined` if no enumerator corresponds to the given value.
             */
            static valueOf(value: number): EndpointSelectionType | undefined;
        }
    }
}
