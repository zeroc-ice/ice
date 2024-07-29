//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        /**
         * Represents the status of a reply.
         */
        class ReplyStatus {
            static readonly Ok: ReplyStatus;
            static readonly UserException: ReplyStatus;
            static readonly ObjectNotExist: ReplyStatus;
            static readonly FacetNotExist: ReplyStatus;
            static readonly OperationNotExist: ReplyStatus;
            static readonly UnknownLocalException: ReplyStatus;
            static readonly UnknownUserException: ReplyStatus;
            static readonly UnknownException: ReplyStatus;

            static valueOf(value: number): ReplyStatus;
            equals(other: any): boolean;
            hashCode(): number;
            toString(): string;

            readonly name: string;
            readonly value: number;
        }
    }
}
