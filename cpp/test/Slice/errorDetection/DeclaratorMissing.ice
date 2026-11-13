// Copyright (c) ZeroC, Inc.
// ice-fmt: ignore-file
module Test
{
    interface Foo
    {
        int bar(string, long l, out bool, out short s);
        int bar2(string s, long, out bool b, out short);
    }
}
