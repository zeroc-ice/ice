// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

class Foo
{
    long l;
    string bar1(string s, int s);
    string bar2(string s, out int s);
    string bar3(out string s, out int s);
    string bar4(string s, int i, out int i);
};

interface IFoo
{
    void op();
    string bar1(string s, int s);
    string bar2(string s, out int s);
    string bar3(out string s, out int s);
    string bar4(string s, int i, out int i);
};
