// Copyright (c) ZeroC, Inc.
#pragma once

[["swift:class-resolver-prefix:IceServantLocator"]]

module Test
{
    exception TestIntfUserException
    {
    }

    exception TestImpossibleException
    {
    }

    interface TestIntf
    {
        void requestFailedException();
        void unknownUserException();
        void unknownLocalException();
        void unknownException();
        void localException();
        void userException();

        void unknownExceptionWithServantException();

        string impossibleException(bool shouldThrow) throws TestImpossibleException;
        string intfUserException(bool shouldThrow) throws TestIntfUserException, TestImpossibleException;

        void shutdown();
    }

    interface TestActivation
    {
        void activateServantLocator(bool activate);
    }
}
