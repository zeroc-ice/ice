// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************



module Test
{

sequence<int> Seq;
sequence<int> Sequence1;
sequence<int> Sequence2;
sequence<int> Sequence3;
interface BarIntf extends Seq { void op(); };
class BarClass1 extends Seq { long l; };
class BarClass2 implements Sequence1, Sequence2, Sequence3 { long l; };
class BarClass3 extends Seq implements Sequence1, Sequence2, Sequence3 { long l; };

};
