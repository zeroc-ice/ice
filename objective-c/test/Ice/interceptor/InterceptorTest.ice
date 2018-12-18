// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

["objc:prefix:TestInterceptor"]
module Test
{

exception InvalidInputException
{
    string reason;
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
    // Raise Ice local exception
    //
    int badSystemAdd(int x, int y);
}

}
