//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cs:typeid-namespace:Ice.servantLocator.TypeId"]]

["cs:namespace:Ice.servantLocator"]
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
    void csException();

    void unknownExceptionWithServantException();

    string impossibleException(bool throw) throws TestImpossibleException;
    string intfUserException(bool throw) throws TestIntfUserException, TestImpossibleException;

    void asyncResponse() throws TestIntfUserException, TestImpossibleException;
    void asyncException() throws TestIntfUserException, TestImpossibleException;

    void shutdown();
}

local class Cookie
{
    ["cpp:const"] string message();
}

interface TestActivation
{
    void activateServantLocator(bool activate);
}

}
