//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        class Object {
            /**
             * Tests whether this object supports a specific Slice interface.
             * @param typeID The type ID of the Slice interface to test against.
             * @param current The Current object for the invocation.
             * @return True if this object has the interface specified by typeID
             * or derives from the interface specified by typeID.
             */
            ice_isA(typeID: string, current: Current): boolean | PromiseLike<boolean>;

            /**
             * Tests whether this object can be reached.
             * @param current The Current object for the invocation.
             */
            ice_ping(current: Current): void | PromiseLike<void>;

            /**
             * Returns the Slice type IDs of the interfaces supported by this object.
             * @param current The Current object for the invocation.
             * @return The Slice type IDs of the interfaces supported by this object, in alphabetical order.
             */
            ice_ids(current: Current): string[] | PromiseLike<string[]>;

            /**
             * Returns the Slice type ID of the most-derived interface supported by this object.
             * @param current The Current object for the invocation.
             * @return The Slice type ID of the most-derived interface.
             */
            ice_id(current: Current): string | PromiseLike<string>;

            /**
             * Dispatches an incoming request and returns the corresponding outgoing response.
             * @param request The incoming request.
             * @return A value task that holds the outgoing response.
             * @remarks Ice marshals any exception thrown by this method into the response.
             */
            dispatch(request: IncomingRequest): OutgoingResponse | PromiseLike<OutgoingResponse>;

            /**
             * Obtains the Slice type ID of this type.
             * @return The return value is always "::Ice::Object".
             */
            static ice_staticId(): string;
        }
    }
}
