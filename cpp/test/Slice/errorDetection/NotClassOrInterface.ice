// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************



module Test
{

sequence<int> Seq;
sequence<int> Sequence1;
sequence<int> Sequence2;
sequence<int> Sequence3;
interface BarIntf extends Seq { void op(); }
class BarClass1 extends Seq { long l; }
class BarClass2 implements Sequence1, Sequence2, Sequence3 { long l; }
class BarClass3 extends Seq implements Sequence1, Sequence2, Sequence3 { long l; }

}
