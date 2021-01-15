//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

sequence<int> Seq;
sequence<int> Sequence1;
sequence<int> Sequence2;
sequence<int> Sequence3;
interface BarIntf : Seq { void op(); }
class BarClass1 : Seq { long l; }
class BarClass2 : Sequence1, Sequence2, Sequence3 { long l; }

}
