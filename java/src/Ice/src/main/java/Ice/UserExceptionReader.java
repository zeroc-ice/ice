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
 * Allows a Dynamic Ice application to instantiate a user exception and
 * intercept its unmarshaling.
 *
 * @see InputStream
 **/
public abstract class UserExceptionReader extends UserException
{
    /**
     * Creates a reader for the given communicator.
     **/
    protected UserExceptionReader(Communicator communicator)
    {
        _communicator = communicator;
    }

    /**
     * Unmarshal an exception from an output stream.
     *
     * @param in The input stream.
     **/
    public abstract void read(Ice.InputStream in);

    @Override
    public void __write(IceInternal.BasicStream os)
    {
        assert(false);
    }

    @Override
    public void __read(IceInternal.BasicStream is)
    {
        InputStream stream = (InputStream)is.closure();
        assert(stream != null);
        read(stream);
    }

    @Override
    public void __write(Ice.OutputStream os)
    {
        assert(false);
    }

    @Override
    public void __read(Ice.InputStream is)
    {
        read(is);
    }

    protected Communicator _communicator;
}
