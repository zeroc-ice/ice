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

sequence<Foo> FooSequence;
dictionary<Foo, Bar> FooBarDictionary;
interface BarIntf extends Foo { void op(); };
class BarClass1 extends Foo { long l; };
class BarClass2 implements Foo1, Foo2, Foo3 { long l; };
class BarClass3 extends Foo implements Foo1, Foo2, Foo3 { long l; };
