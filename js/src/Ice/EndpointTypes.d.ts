//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export namespace Ice
{
    /**
     * Determines the order in which the Ice run time uses the endpoints
     * in a proxy when establishing a connection.
     */
    class EndpointSelectionType
    {
        /**
         * <code>Random</code> causes the endpoints to be arranged in a random order.
         */
        static readonly Random:EndpointSelectionType;
        /**
         * <code>Ordered</code> forces the Ice run time to use the endpoints in the
         * order they appeared in the proxy.
         */
        static readonly Ordered:EndpointSelectionType;
        static valueOf(value:number):EndpointSelectionType;
        equals(other:any):boolean;
        hashCode():number;
        toString():string;
        readonly name:string;
        readonly value:number;
    }
}
