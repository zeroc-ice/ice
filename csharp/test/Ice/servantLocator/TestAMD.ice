// Copyright (c) ZeroC, Inc.

#pragma once

["cs:namespace:Ice.servantLocator.AMD"]
module Test
{

exception TestIntfUserException
{
}

exception TestImpossibleException
{
}

["amd"] interface TestIntf
{
    void requestFailedException();
    void unknownUserException();
    void unknownLocalException();
    void unknownException();
    void localException();
    void userException();
    void csException();

    void unknownExceptionWithServantException();

    string impossibleException(["cs:identifier:shouldThrow"] bool throw) throws TestImpossibleException;
    string intfUserException(["cs:identifier:shouldThrow"] bool throw) throws TestIntfUserException, TestImpossibleException;

    void asyncResponse() throws TestIntfUserException, TestImpossibleException;
    void asyncException() throws TestIntfUserException, TestImpossibleException;

    void shutdown();
}

interface TestActivation
{
    void activateServantLocator(bool activate);
}

}
