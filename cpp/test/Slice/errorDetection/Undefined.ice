//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

sequence<Foo> FooSequence;
dictionary<Foo, Bar> FooBarDictionary;
interface BarIntf extends Foo { void op(); }
class BarClass1 extends Foo { long l; }

}
