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
    void f3(out Seq*);

    Dict* f4();
    void f5(Dict*);
    void f6(out Dict*);
    void op();
};
