//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["swift:class-resolver-prefix:IceServantLocatorAMD"]]

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

    void unknownExceptionWithServantException();

    string impossibleException(bool throw) throws TestImpossibleException;
    string intfUserException(bool throw) throws TestIntfUserException, TestImpossibleException;

    void asyncResponse() throws TestIntfUserException, TestImpossibleException;
    void asyncException() throws TestIntfUserException, TestImpossibleException;

    void shutdown();
}

["amd"] interface TestActivation
{
    void activateServantLocator(bool activate);
}

}
