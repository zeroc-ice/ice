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

module Module1 { };
module Module2 { };
module Module3 { };
module Mod
{

sequence<Mod> Seq;
dictionary<int, Mod> Dict;
interface BarIntf extends Mod { void op(); };
class BarClass1 extends Mod { long l; };
class BarClass2 implements Module1, Module2, Module3 { long l; };
class BarClass3 extends Mod implements Module1, Module2, Module3 { long l; };

};

exception E { };

struct S
{
    E e;
};

interface I
{
    E foo(E e1; E e2);
    void op();
};
