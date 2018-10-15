// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

declare module "ice"
{
    namespace Ice
    {
        /**
         * The base class for instances of Slice classes.
         */
        class Value
        {
            /**
             * The Ice run time invokes this method prior to marshaling an object's data members. This allows a subclass
             * to override this method in order to validate its data members.
             */
            ice_preMarshal():void;

            /**
             * The Ice run time invokes this method vafter unmarshaling an object's data members. This allows a
             * subclass to override this method in order to perform additional initialization.
             */
            ice_postUnmarshal():void;

            /**
             * Obtains the Slice type ID of the most-derived class supported by this object.
             * @return The type ID.
             */
            ice_id():string;

            /**
             * Obtains the Slice type ID of this type.
             * @return The return value is always "::Ice::Object".
             */
            static ice_staticId():string;

            /**
             * Returns a shallow copy of the object.
             * @return The cloned value.
             */
            ice_clone():this;

            /**
             * Obtains the sliced data associated with this instance.
             * @return The sliced data if the value has a preserved-slice base class and has been sliced during
             * unmarshaling of the value, nil otherwise.
             */
            ice_getSlicedData():SlicedData;
        }

        class InterfaceByValue extends Value
        {
            constructor(id:string);
        }
    }
}
