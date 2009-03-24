// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.custom14;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import test.Ice.custom14.Test.BoolSeqHolder;
import test.Ice.custom14.Test.ByteSeqHolder;
import test.Ice.custom14.Test.C;
import test.Ice.custom14.Test.CArrayHolder;
import test.Ice.custom14.Test.CListHolder;
import test.Ice.custom14.Test.CSeqHolder;
import test.Ice.custom14.Test.DSeqHolder;
import test.Ice.custom14.Test.DoubleSeqHolder;
import test.Ice.custom14.Test.ESeqHolder;
import test.Ice.custom14.Test.FloatSeqHolder;
import test.Ice.custom14.Test.IntSeqHolder;
import test.Ice.custom14.Test.LongSeqHolder;
import test.Ice.custom14.Test.SSeqHolder;
import test.Ice.custom14.Test.ShortSeqHolder;
import test.Ice.custom14.Test.StringSeqHolder;
import test.Ice.custom14.Test.StringSeqSeqHolder;
import test.Ice.custom14.Test.TestIntf;

public final class TestI extends TestIntf
{
    public
    TestI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    public List
    opCArray(List inSeq, CArrayHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List
    opCList(List inSeq, CListHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public C[]
    opCSeq(C[] inSeq, CSeqHolder outSeq, Ice.Current current)
    {
        seq = new ArrayList(Arrays.asList(inSeq));
        outSeq.value = new C[seq.size()];
        seq.toArray(outSeq.value);
        return outSeq.value;
    }

    public List
    opBoolSeq(List inSeq, BoolSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List
    opByteSeq(List inSeq, ByteSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List
    opDSeq(List inSeq, DSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List
    opDoubleSeq(List inSeq, DoubleSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List
    opESeq(List inSeq, ESeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List
    opFloatSeq(List inSeq, FloatSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List
    opIntSeq(List inSeq, IntSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List
    opLongSeq(List inSeq, LongSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List
    opSSeq(List inSeq, SSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List
    opShortSeq(List inSeq, ShortSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List
    opStringSeq(List inSeq, StringSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List[]
    opStringSeqSeq(List[] inSeq, StringSeqSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public void
    shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    private Ice.Communicator _communicator;
}
