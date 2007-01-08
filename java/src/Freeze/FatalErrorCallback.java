// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

public interface FatalErrorCallback
{
    void handleError(Evictor evictor, Ice.Communicator communicator, RuntimeException ex);
}
