// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * The base class for servants.
         */
        class Object {
            /**
             * Tests whether this object supports a specific Slice interface.
             *
             * @param typeID The type ID of the Slice interface to test against.
             * @param current The Current object for the invocation.
             *
             * @returns True if this object has the interface specified by typeID or derives from the interface
             * specified by typeID.
             */
            ice_isA(typeID: string, current: Current): boolean | PromiseLike<boolean>;

            /**
             * Tests whether this object can be reached.
             *
             * @param current The Current object for the invocation.
             */
            ice_ping(current: Current): void | PromiseLike<void>;

            /**
             * Returns the Slice type IDs of the interfaces supported by this object.
             *
             * @param current The Current object for the invocation.
             * @returns The Slice type IDs of the interfaces supported by this object, in alphabetical order.
             */
            ice_ids(current: Current): string[] | PromiseLike<string[]>;

            /**
             * Returns the Slice type ID of the most-derived interface supported by this object.
             *
             * @param current The Current object for the invocation.
             * @returns The Slice type ID of the most-derived interface.
             */
            ice_id(current: Current): string | PromiseLike<string>;

            /**
             * Dispatches an incoming request and returns the corresponding outgoing response.
             *
             * @param request The incoming request.
             * @returns The outgoing response or a promise that is resolved with the outgoing response.
             * @remarks Ice marshals any exception thrown by this method into the response.
             */
            dispatch(request: IncomingRequest): OutgoingResponse | PromiseLike<OutgoingResponse>;

            /**
             * Returns the Slice type id of the interface or class associated with this proxy class.
             *
             * @returns The type id, "::Ice::Object".
             */
            static ice_staticId(): string;
        }
    }
}
