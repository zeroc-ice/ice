//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

sequence<Foo> FooSequence;
dictionary<Foo, Bar> FooBarDictionary;
interface BarIntf : Foo { void op(); }
class BarClass1 : Foo { long l; }

}
