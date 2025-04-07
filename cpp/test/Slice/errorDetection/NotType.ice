// Copyright (c) ZeroC, Inc.

module Test
{
    module MyModule {}

    exception E
    {
        MyModule exceptionField;
    }

    module Mod
    {
        sequence<MyModule> Seq;
        dictionary<int, Mod> Dict;
        interface BarIntf extends MyModule, E { void op(); }
        class BarClass1 extends Mod { E l; }
    }

    struct S
    {
        E structField;
    }

    interface I
    {
        E foo(E e1, E e2);
        S op(S s1, S s2);
    }
}
