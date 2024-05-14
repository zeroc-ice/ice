//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice"
{
    namespace Ice
    {
        /**
         * Base class for all Ice exceptions.
         */
        abstract class Exception extends Error
        {
            /**
             * Returns the type id of this exception.
             *
             * @return The type id of this exception.
             **/
            ice_id():string;

            /**
             * Returns a string representation of this exception.
             *
             * @return A string representation of this exception.
             **/
            toString():string;

            ice_cause:string|Error;
        }

        /**
         * Base class for all Ice run-time exceptions.
         */
        abstract class LocalException extends Exception {}

        /**
         * Base class for all Ice user exceptions.
         */
        abstract class UserException extends Exception
        {
            /**
             * Obtains the Slice type ID of this exception.
             * @return The fully-scoped type ID.
             */
            static ice_staticId():string;
        }
    }
}
