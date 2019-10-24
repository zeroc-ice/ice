//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    internal sealed class ReplyStatus
    {
        public const byte replyOK = 0;
        public const byte replyUserException = 1;
        public const byte replyObjectNotExist = 2;
        public const byte replyFacetNotExist = 3;
        public const byte replyOperationNotExist = 4;
        public const byte replyUnknownLocalException = 5;
        public const byte replyUnknownUserException = 6;
        public const byte replyUnknownException = 7;

        private ReplyStatus()
        {
        }
    }
}
