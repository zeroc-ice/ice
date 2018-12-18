// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    sealed class ReplyStatus
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
