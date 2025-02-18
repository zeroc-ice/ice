// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Ice.servantLocator.AMD"]]
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
    //void userException();
    void javaException();

    void unknownExceptionWithServantException();

    // TODO rename the throw variable in all language mappings before adding more 'xxx:identifier'.
    string impossibleException(["java:identifier:shouldThrow"] bool throw) throws TestImpossibleException;
    string intfUserException(["java:identifier:shouldThrow"] bool throw) throws TestIntfUserException, TestImpossibleException;

    void asyncResponse() throws TestIntfUserException, TestImpossibleException;
    void asyncException() throws TestIntfUserException, TestImpossibleException;

    void shutdown();
}

interface TestActivation
{
    void activateServantLocator(bool activate);
}

}
