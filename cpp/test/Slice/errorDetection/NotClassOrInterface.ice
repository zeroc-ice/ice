// Copyright (c) ZeroC, Inc.

module Test
{
    sequence<int> Seq;
    sequence<int> Sequence1;
    sequence<int> Sequence2;
    sequence<int> Sequence3;
    interface BarIntf extends Seq { void op(); }
    class BarClass1 extends Seq { long l; }
}
