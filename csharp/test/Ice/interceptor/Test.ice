//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning:reserved-identifier]]

module ZeroC::Ice::Test::Interceptor
{

struct Token
{
    long expiration;
    string hash;
}

exception InvalidInputException
{
    string message;
}

interface MyObject
{
    // A simple addition
    int add(int x, int y);

    // Will throw RetryException until current.Context["retry"] is "no"
    int addWithRetry(int x, int y);

    // Throws remote exception
    int badAdd(int x, int y);
    // Throws ONE
    int notExistAdd(int x, int y);
    void opWithBinaryContext(Token token);

    // AMD version of the above:

    // Simple add
    [amd] int amdAdd(int x, int y);

    // Will throw RetryException until current.Context["retry"] is "no"
    [amd] int amdAddWithRetry(int x, int y);

    // Throws remote exception
    [amd] int amdBadAdd(int x, int y);

    // Throws ONE
    [amd] int amdNotExistAdd(int x, int y);
}
}
