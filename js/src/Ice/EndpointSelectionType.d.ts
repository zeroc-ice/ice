// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Determines the order in which the Ice runtime uses the endpoints in a proxy when establishing a connection.
         */
        class EndpointSelectionType extends Ice.EnumBase {
            /**
             * `Random` arranges the endpoints in a random order.
             */
            static readonly Random: EndpointSelectionType;

            /**
             * `Ordered` forces the Ice runtime to use the endpoints in the order they appear in the proxy.
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
