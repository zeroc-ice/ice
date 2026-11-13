// Copyright (c) ZeroC, Inc.

// ice-fmt: ignore-file

module Test
{
    class Class1 { byte b; long l; }
    class Class1;
    class Class1 { long l; }

    class Class2 { byte b; }
    class Class2;
    class Class2 { byte b; }
}
