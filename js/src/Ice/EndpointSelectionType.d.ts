//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        /**
         * Determines the order in which the Ice run time uses the endpoints in a proxy when establishing a connection.
         */
        class EndpointSelectionType extends Ice.EnumBase {
            /**
             * <code>Random</code> causes the endpoints to be arranged in a random order.
             */
            static readonly Random: EndpointSelectionType;

            /**
             * <code>Ordered</code> forces the Ice run time to use the endpoints in the order they appeared in the proxy.
             */
            static readonly Ordered: EndpointSelectionType;

            /**
             * Returns the enumerator for the given value.
             *
             * @param value The enumerator value.
             * @returns The enumerator for the given value.
             */
            static valueOf(value: number): EndpointSelectionType;
        }
    }
}
