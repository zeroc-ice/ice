// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

interface Foo
{
    void op() throws UndefinedException;
    void op2() throws class;
};

sequence<int> IntSeq;

struct S
{
    int i;
};

dictionary<string, string> StringDict;

enum E { red };

const int x = 1;
