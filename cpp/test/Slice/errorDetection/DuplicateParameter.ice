// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

class Foo
{
    string bar1(string s, int s);
    string bar2(string s; int s);
    string bar3(; string s, int s);
    string bar4(string s, int i; int i);
};

interface IFoo
{
    string bar1(string s, int s);
    string bar2(string s; int s);
    string bar3(; string s, int s);
    string bar4(string s, int i; int i);
};
