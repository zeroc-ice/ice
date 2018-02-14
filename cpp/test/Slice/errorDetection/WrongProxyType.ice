// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************



module Test
{

sequence<int> Seq;
dictionary<Seq, int> Dict;

interface I
{
    Seq* f1();
    void f2(Seq*);
    void f3(out Seq*);

    Dict* f4();
    void f5(Dict*);
    void f6(out Dict*);
    void op();
};

};
