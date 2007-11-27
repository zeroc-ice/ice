// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public interface OutgoingAsyncMessageCallback
{
    void __sent(Ice.ConnectionI connection);
    void __finished(Ice.LocalException ex);
};