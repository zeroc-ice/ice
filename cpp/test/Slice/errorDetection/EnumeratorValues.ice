// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module Test
{

enum Enum1
{
    A = 0,
    B = 2147483647,
    C,
    D = 2147483649,
    E = -1,
    F = -2147483649
}

}
