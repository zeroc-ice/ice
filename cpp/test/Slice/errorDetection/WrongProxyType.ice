// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


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
