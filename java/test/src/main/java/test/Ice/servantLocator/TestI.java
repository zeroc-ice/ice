// Copyright (c) ZeroC, Inc.

package test.Ice.servantLocator;

import test.Ice.servantLocator.Test.TestImpossibleException;
import test.Ice.servantLocator.Test.TestIntf;
import test.Ice.servantLocator.Test.TestIntfUserException;

public final class TestI implements TestIntf {
    @Override
    public void requestFailedException(com.zeroc.Ice.Current current) {}

    @Override
    public void unknownUserException(com.zeroc.Ice.Current current) {}

    @Override
    public void unknownLocalException(com.zeroc.Ice.Current current) {}

    @Override
    public void unknownException(com.zeroc.Ice.Current current) {}

    @Override
    public void localException(com.zeroc.Ice.Current current) {}

    //     public void userException(com.zeroc.Ice.Current current)
    //     {
    //     }

    @Override
    public void javaException(com.zeroc.Ice.Current current) {}

    @Override
    public void unknownExceptionWithServantException(com.zeroc.Ice.Current current) {
        throw new com.zeroc.Ice.ObjectNotExistException();
    }

    @Override
    public String impossibleException(boolean shouldThrow, com.zeroc.Ice.Current current)
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
    public String intfUserException(boolean shouldThrow, com.zeroc.Ice.Current current)
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
    public void asyncResponse(com.zeroc.Ice.Current current)
            throws TestIntfUserException, TestImpossibleException {
        //
        // Only relevant for AMD.
        //
    }

    @Override
    public void asyncException(com.zeroc.Ice.Current current)
            throws TestIntfUserException, TestImpossibleException {
        //
        // Only relevant for AMD.
        //
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current) {
        current.adapter.getCommunicator().shutdown();
    }
}
