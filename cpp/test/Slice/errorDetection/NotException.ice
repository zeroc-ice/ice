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
