// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

sequence<int> Seq;
sequence<int> Sequence1;
sequence<int> Sequence2;
sequence<int> Sequence3;
interface BarIntf extends Seq { void op(); };
class BarClass1 extends Seq { long l; };
class BarClass2 implements Sequence1, Sequence2, Sequence3 { long l; };
class BarClass3 extends Seq implements Sequence1, Sequence2, Sequence3 { long l; };
