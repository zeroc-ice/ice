// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

sequence<Foo> FooSequence;
dictionary<Foo, Bar> FooBarDictionary;
interface BarIntf extends Foo { void op(); };
class BarClass1 extends Foo { long l; };
class BarClass2 implements Foo1, Foo2, Foo3 { long l; };
class BarClass3 extends Foo implements Foo1, Foo2, Foo3 { long l; };
