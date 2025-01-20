// Copyright (c) ZeroC, Inc.

#pragma once

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
    void stdException();
    void cppException();

    void unknownExceptionWithServantException();

    // TODO rename the throw variable in all language mappings before adding more 'xxx:identifier'.
    string impossibleException(["cpp:identifier:shouldThrow"] bool throw) throws TestImpossibleException;
    string intfUserException(["cpp:identifier:shouldThrow"] bool throw) throws TestIntfUserException, TestImpossibleException;

    void asyncResponse() throws TestIntfUserException, TestImpossibleException;
    void asyncException() throws TestIntfUserException, TestImpossibleException;

    void shutdown();
}

interface TestActivation
{
    void activateServantLocator(bool activate);
}

}
