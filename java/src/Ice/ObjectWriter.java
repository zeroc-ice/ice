// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class ObjectWriter extends ObjectImpl
{
    public abstract void write(OutputStream out);

    public void
    __write(IceInternal.BasicStream os)
    {
        IceInternal.BasicOutputStream bos = (IceInternal.BasicOutputStream)os;
        write(bos._out);
    }

    public void
    __read(IceInternal.BasicStream is, boolean rid)
    {
        assert(false);
    }
}
