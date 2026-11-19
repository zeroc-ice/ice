// Copyright (c) ZeroC, Inc.

// icefmt: ignore-file

module Test
{
    exception Base {}
    exception Base { int i; }
    exception Derived extends Base { int j; }
    exception Derived { int j; }
}
