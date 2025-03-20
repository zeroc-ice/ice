// Copyright (c) ZeroC, Inc.

module Test
{
    exception module;
    exception void
    {
        int i;
    }

    struct exception;
    struct class
    {
        int i;
    }

    class idempotent;
    class struct
    {
        int i;
    }

    interface interface;
    interface extends
    {
        void f();
    }

    interface Foo
    {
        void module();

        int bar(string throws, long l, out bool void, out short s);

        int bar2(string s, long byte, out bool b, out short short);
    }

    class Bar
    {
        string int;
        float long;
        double float;
        string double;
    }

    sequence<int> Object;

    dictionary<int, int> dictionary;

    enum string
    {
        a, b, c
    }

    enum E
    {
        sequence,
        dictionary,
        enum,
        laksdjflkj
    }

    interface Bletch extends Object
    {
    }
}
