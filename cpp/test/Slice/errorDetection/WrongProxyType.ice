// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

sequence<int> Seq;
dictionary<Seq, int> Dict;

interface I
{
    Seq* f1();
    void f2(Seq*);
    void f3(; Seq*);

    Dict* f1();
    void f2(Dict*);
    void f3(; Dict*);
    void op();
};
