// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public interface OutgoingMessageCallback
{
    boolean send(Ice.ConnectionI conection, boolean compress, boolean response)
        throws LocalExceptionWrapper;

    void sent();
    void finished(Ice.LocalException ex, boolean sent);
}
