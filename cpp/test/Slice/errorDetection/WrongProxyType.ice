// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

sequence<int> Sequence;
dictionary<Sequence, int> Dictionary;

interface I
{
    Sequence* f1();
    void f2(Sequence*);
    void f3(; Sequence*);

    Dictionary* f1();
    void f2(Dictionary*);
    void f3(; Dictionary*);
};
