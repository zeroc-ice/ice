// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
