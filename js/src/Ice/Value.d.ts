// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * The base class for instances of Slice classes.
         */
        class Value {
            /**
             * The Ice run time invokes this method prior to marshaling an object's data members. This allows a subclass
             * to override this method in order to validate its data members.
             */
            ice_preMarshal(): void;

            /**
             * The Ice run time invokes this method after unmarshaling an object's data members. This allows a
             * subclass to override this method in order to perform additional initialization.
             */
            ice_postUnmarshal(): void;

            /**
             * Obtains the Slice type ID of the most-derived class supported by this object.
             *
             * @returns The type ID.
             */
            ice_id(): string;

            /**
             * Obtains the Slice type ID of this type.
             *
             * @returns The return value is always "::Ice::Object".
             */
            static ice_staticId(): string;

            /**
             * Obtains the sliced data associated with this instance.
             *
             * @returns The sliced data if the value has a preserved-slice base class and has been sliced during
             * unmarshaling of the value, nil otherwise.
             */
            ice_getSlicedData(): SlicedData;
        }
    }
}
