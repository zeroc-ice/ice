// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

sequence<int> Sequence;
sequence<int> Sequence1;
sequence<int> Sequence2;
sequence<int> Sequence3;
interface BarIntf extends Sequence { };
class BarClass1 extends Sequence { };
class BarClass2 implements Sequence1, Sequence2, Sequence3 { };
class BarClass3 extends Sequence implements Sequence1, Sequence2, Sequence3 { };
