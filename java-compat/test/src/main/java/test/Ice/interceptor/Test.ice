//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["java:package:test.Ice.interceptor"]]
module Test
{

exception InvalidInputException
{
    string message;
}

local exception RetryException
{
}

interface MyObject
{
    //
    // A simple addition
    //
    int add(int x, int y);

    //
    // Will throw RetryException until current.ctx["retry"] is "no"
    //
    int addWithRetry(int x, int y);

    //
    // Raise user exception
    //
    int badAdd(int x, int y) throws InvalidInputException;

    //
    // Raise ONE
    //
    int notExistAdd(int x, int y);

    //
    // Raise system exception
    //
    int badSystemAdd(int x, int y);

    //
    // AMD version of the above:
    //

    //
    // Simple add
    //
    ["amd"] int amdAdd(int x, int y);

    //
    // Will throw RetryException until current.ctx["retry"] is "no"
    //
    ["amd"] int amdAddWithRetry(int x, int y);

    //
    // Raise user exception
    //
    ["amd"] int amdBadAdd(int x, int y) throws InvalidInputException;

    //
    // Raise ONE
    //
    ["amd"] int amdNotExistAdd(int x, int y);

    //
    // Raise system exception
    //
    ["amd"] int amdBadSystemAdd(int x, int y);

}

}
