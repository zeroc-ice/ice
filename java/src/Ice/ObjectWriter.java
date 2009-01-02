// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
        OutputStream stream = (OutputStream)os.closure();
        write(stream);
    }

    public void
    __read(IceInternal.BasicStream is, boolean rid)
    {
        assert(false);
    }
}
