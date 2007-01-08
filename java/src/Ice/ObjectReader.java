// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        IceInternal.BasicInputStream bis = (IceInternal.BasicInputStream)is;
        read(bis._in, rid);
    }
}
