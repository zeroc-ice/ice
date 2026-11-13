// Copyright (c) ZeroC, Inc.

// ice-fmt: ignore-file

module Test
{
    sequence<int> Seq;
    dictionary<bool, int> Dict;

    interface I
    {
        Seq* f1();
        void f2(Seq* s);
        void f3(out Seq* s);

        Dict* f4();
        void f5(Dict* s);
        void f6(out Dict* s);
        void op();
    }
}
