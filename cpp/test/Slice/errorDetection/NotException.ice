// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************






module M
{

class C { long l; };
sequence<int> S;
dictionary<string, string> D;

interface Bar
{
    void foo1() throws C;
    void foo2() throws S;
    void foo3() throws D;
    void foo4() throws int;
    void op();
};

};
