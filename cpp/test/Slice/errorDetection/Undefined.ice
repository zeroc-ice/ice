// Copyright (c) ZeroC, Inc.

// icefmt: ignore-file

module Test
{
    sequence<Fake> FooSequence;
    dictionary<Foo, Bar> FooBarDictionary;
    interface BarIntf extends Ping { void op(); }
    class BarClass1 extends Pong { long l; }
}
