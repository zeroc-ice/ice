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

interface Foo
{
    int bar(string, long l, out bool, out short s);
    int bar2(string s, long, out bool b, out short);
}

}
