//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:LIBRARY_TEST_API"]]

module Test
{

exception UserError
{
    string message;
}

interface MyInterface
{
    void op(bool throwIt) throws UserError;
}

}
