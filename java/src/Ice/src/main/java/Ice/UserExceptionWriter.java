// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Allows a Dynamic Ice application to wrap a native exception and
 * intercept its marshaling.
 *
 * @see OutputStream
 **/
public abstract class UserExceptionWriter extends UserException
{
    /**
     * Creates a writer for the given communicator.
     **/
    public UserExceptionWriter(Communicator communicator)
    {
        _communicator = communicator;
    }

    /**
     * Marshal the encapsulated exception into an output stream.
     *
     * @param os The output stream.
     **/
    public abstract void
    write(Ice.OutputStream os);

    @Override
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

    @Override
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

    protected Communicator _communicator;
}
