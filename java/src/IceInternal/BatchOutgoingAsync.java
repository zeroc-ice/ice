// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class BatchOutgoingAsync extends OutgoingAsyncMessageCallback
{
    public final void
    __sent(final Ice.ConnectionI connection)
    {
        __releaseCallback();
    }
    
    public final void
    __finished(Ice.LocalException exc)
    {
        __exception(exc);
    }

}
