// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

sequence<Foo> FooSequence;
dictionary<Foo, Bar> FooBarDictionary;
interface BarIntf extends Foo { };
class BarClass1 extends Foo { };
class BarClass2 implements Foo1, Foo2, Foo3 { };
class BarClass3 extends Foo implements Foo1, Foo2, Foo3 { };
