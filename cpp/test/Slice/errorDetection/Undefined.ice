//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

sequence<Foo> FooSequence;
dictionary<Foo, Bar> FooBarDictionary;
interface BarIntf : Foo { void op(); }
class BarClass1 : Foo { long l; }
class BarClass2 implements Foo1, Foo2, Foo3 { long l; }
class BarClass3 : Foo implements Foo1, Foo2, Foo3 { long l; }

}
