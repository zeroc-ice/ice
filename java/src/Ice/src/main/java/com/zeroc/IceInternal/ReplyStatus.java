// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceInternal;

interface ReplyStatus
{
    static final byte replyOK = 0;
    static final byte replyUserException = 1;
    static final byte replyObjectNotExist = 2;
    static final byte replyFacetNotExist = 3;
    static final byte replyOperationNotExist = 4;
    static final byte replyUnknownLocalException = 5;
    static final byte replyUnknownUserException = 6;
    static final byte replyUnknownException = 7;
}
