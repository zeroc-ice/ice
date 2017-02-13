// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************



module Test
{

interface Foo
{
    void bar(long l1, out long l2, long l3);
    void baz(out long l2, long l3);
};

};
