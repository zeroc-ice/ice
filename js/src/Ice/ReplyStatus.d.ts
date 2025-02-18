// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Represents the status of a reply.
         */
        class ReplyStatus {
            /**
             * A successful reply message.
             */
            static readonly Ok: ReplyStatus;

            /**
             * A user exception reply message.
             */
            static readonly UserException: ReplyStatus;

            /**
             * The target object does not exist.
             */
            static readonly ObjectNotExist: ReplyStatus;

            /**
             * The target object does not support the facet.
             */
            static readonly FacetNotExist: ReplyStatus;

            /**
             * The target object does not support the operation.
             */
            static readonly OperationNotExist: ReplyStatus;

            /**
             * The dispatch failed with an Ice local exception
             */
            static readonly UnknownLocalException: ReplyStatus;

            /**
             * The dispatch failed with a Slice user exception that does not conform to the exception specification of
             * the operation
             */
            static readonly UnknownUserException: ReplyStatus;

            /**
             * The dispatch failed with some other exception.
             */
            static readonly UnknownException: ReplyStatus;

            /**
             * Returns the enumerator for the given value.
             *
             * @param value The enumerator value.
             * @returns The enumerator for the given value.
             */
            static valueOf(value: number): ReplyStatus;
        }
    }
}
