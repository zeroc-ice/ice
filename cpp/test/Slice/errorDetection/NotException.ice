// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

module M
{

class C { };
sequence<int> S;
dictionary<string, string> D;

interface Bar
{
    void foo() throws C, S, D, int;
};

};
