// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class ObjectReader extends ObjectImpl
{
    public abstract void read(InputStream in, boolean rid);

    public void
    __write(IceInternal.BasicStream os)
    {
        assert(false);
    }

    public void
    __read(IceInternal.BasicStream is, boolean rid)
    {
        InputStream stream = (InputStream)is.closure();
        read(stream, rid);
    }
}
