// Copyright (c) ZeroC, Inc.

package test.Ice.servantLocator;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectNotExistException;

import test.Ice.servantLocator.Test.TestImpossibleException;
import test.Ice.servantLocator.Test.TestIntf;
import test.Ice.servantLocator.Test.TestIntfUserException;

public final class TestI implements TestIntf {
    @Override
    public void requestFailedException(Current current) {
    }

    @Override
    public void unknownUserException(Current current) {
    }

    @Override
    public void unknownLocalException(Current current) {
    }

    @Override
    public void unknownException(Current current) {
    }

    @Override
    public void localException(Current current) {
    }

    //     public void userException(com.zeroc.Ice.Current current)
    //     {
    //     }

    @Override
    public void javaException(Current current) {
    }

    @Override
    public void unknownExceptionWithServantException(Current current) {
        throw new ObjectNotExistException();
    }

    @Override
    public String impossibleException(boolean shouldThrow, Current current)
        throws TestImpossibleException {
        if (shouldThrow) {
            throw new TestImpossibleException();
        }

        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello";
    }

    @Override
    public String intfUserException(boolean shouldThrow, Current current)
        throws TestIntfUserException, TestImpossibleException {
        if (shouldThrow) {
            throw new TestIntfUserException();
        }

        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello";
    }

    @Override
    public void asyncResponse(Current current)
        throws TestIntfUserException, TestImpossibleException {
        //
        // Only relevant for AMD.
        //
    }

    @Override
    public void asyncException(Current current)
        throws TestIntfUserException, TestImpossibleException {
        //
        // Only relevant for AMD.
        //
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }
}
