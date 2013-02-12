// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.servantLocator;

import test.Ice.servantLocator.Test.TestImpossibleException;
import test.Ice.servantLocator.Test.TestIntfUserException;
import test.Ice.servantLocator.Test._TestIntfDisp;

public final class TestI extends _TestIntfDisp
{
    public void 
    requestFailedException(Ice.Current current)
    {
    }

    public void 
    unknownUserException(Ice.Current current)
    {
    }

    public void 
    unknownLocalException(Ice.Current current)
    {
    }

    public void 
    unknownException(Ice.Current current)
    {
    }

    public void 
    localException(Ice.Current current)
    {
    }

//     public void 
//     userException(Ice.Current current)
//     {
//     }

    public void 
    javaException(Ice.Current current)
    {
    }

    public void 
    unknownExceptionWithServantException(Ice.Current current)
    {
        throw new Ice.ObjectNotExistException();
    }

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

    public void
    asyncResponse(Ice.Current current) throws TestIntfUserException, TestImpossibleException
    {
        //
        // Only relevant for AMD.
        //
    }

    public void
    asyncException(Ice.Current current) throws TestIntfUserException, TestImpossibleException
    {
        //
        // Only relevant for AMD.
        //
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
