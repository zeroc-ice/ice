// Copyright (c) ZeroC, Inc.

module Test
{
    interface Foo
    {
        void bar(long l1, out long l2, long l3);
        void baz(out long l2, long l3);
        void bab(out out long l1, long l2, out long l3);
    }
}
