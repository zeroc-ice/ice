//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module Test
{

exception InvalidInputException
{
    string message;
}

interface MyObject
{
//
// A simple addition
//
    int add(int x, int y);

//
// Raise user exception
//
    int badAdd(int x, int y) throws InvalidInputException;

//
// Raise ONE
//
    int notExistAdd(int x, int y);

//
// AMD version of the above:
//

//
// Simple add
//
    ["amd"] int amdAdd(int x, int y);

//
// Raise user exception
//
    ["amd"] int amdBadAdd(int x, int y) throws InvalidInputException;

//
// Raise ONE
//
    ["amd"] int amdNotExistAdd(int x, int y);
}

}

