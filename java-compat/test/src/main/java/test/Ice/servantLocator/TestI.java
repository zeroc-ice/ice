//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.servantLocator;

import test.Ice.servantLocator.Test.TestImpossibleException;
import test.Ice.servantLocator.Test.TestIntfUserException;
import test.Ice.servantLocator.Test._TestIntfDisp;

public final class TestI extends _TestIntfDisp
{
    @Override
    public void
    requestFailedException(Ice.Current current)
    {
    }

    @Override
    public void
    unknownUserException(Ice.Current current)
    {
    }

    @Override
    public void
    unknownLocalException(Ice.Current current)
    {
    }

    @Override
    public void
    unknownException(Ice.Current current)
    {
    }

    @Override
    public void
    localException(Ice.Current current)
    {
    }

//     public void
//     userException(Ice.Current current)
//     {
//     }

    @Override
    public void
    javaException(Ice.Current current)
    {
    }

    @Override
    public void
    unknownExceptionWithServantException(Ice.Current current)
    {
        throw new Ice.ObjectNotExistException();
    }

    @Override
    public String
    impossibleException(boolean _throw, Ice.Current current) throws TestImpossibleException
    {
        if(_throw)
        {
            throw new TestImpossibleException();
        }

        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello";
    }

    @Override
    public String
    intfUserException(boolean _throw, Ice.Current current) throws TestIntfUserException, TestImpossibleException
    {
        if(_throw)
        {
            throw new TestIntfUserException();
        }

        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello";
    }

    @Override
    public void
    asyncResponse(Ice.Current current) throws TestIntfUserException, TestImpossibleException
    {
        //
        // Only relevant for AMD.
        //
    }

    @Override
    public void
    asyncException(Ice.Current current) throws TestIntfUserException, TestImpossibleException
    {
        //
        // Only relevant for AMD.
        //
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
