// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class UserExceptionWriter extends UserException
{
    public UserExceptionWriter(Communicator communicator)
    {
        _communicator = communicator;
    }

    public abstract void
    write(Ice.OutputStream os);

    public abstract boolean
    usesClasses();

    public void
    __write(IceInternal.BasicStream os)
    {
        OutputStream stream = (OutputStream)os.closure();
        if(stream == null)
        {
            stream = new OutputStreamI(_communicator, os);
        }
        write(stream);
    }

    public void
    __read(IceInternal.BasicStream is, boolean rid)
    {
        assert(false);
    }

    public void
    __write(Ice.OutputStream os)
    {
        write(os);
    }

    public void
    __read(Ice.InputStream is, boolean rid)
    {
        assert(false);
    }

    public boolean
    __usesClasses()
    {
        return usesClasses();
    }

    protected Communicator _communicator;
}
