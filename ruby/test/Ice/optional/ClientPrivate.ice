// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Test.ice>

module Test
{

//
// The server doesn't know this class.
//
class D extends B
{
    string ds;
    optional(990) StringSeq seq;
    optional(1000) A ao;
}

//
// This class is a hack that allows us to invoke the opClassAndUnknownOptional operation
// on the server and pass an optional argument. This isn't necessary in other language
// mappings where the public stream API is available.
//
interface Initial2
{
    void opClassAndUnknownOptional(A p, optional(1) Object o);

    void opVoid(optional(1) int a, optional(2) string v);
}

}
