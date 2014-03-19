// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.custom;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

import test.Ice.custom.Test.BoolSeqHolder;
import test.Ice.custom.Test.ByteSeqHolder;
import test.Ice.custom.Test.C;
import test.Ice.custom.Test.CArrayHolder;
import test.Ice.custom.Test.CListHolder;
import test.Ice.custom.Test.CSeqHolder;
import test.Ice.custom.Test.DSeqHolder;
import test.Ice.custom.Test.DoubleSeqHolder;
import test.Ice.custom.Test.E;
import test.Ice.custom.Test.ESeqHolder;
import test.Ice.custom.Test.FloatSeqHolder;
import test.Ice.custom.Test.IntSeqHolder;
import test.Ice.custom.Test.LongSeqHolder;
import test.Ice.custom.Test.S;
import test.Ice.custom.Test.SSeqHolder;
import test.Ice.custom.Test.ShortSeqHolder;
import test.Ice.custom.Test.StringSeqHolder;
import test.Ice.custom.Test.StringSeqSeqHolder;
import test.Ice.custom.Test.TestIntf;

public final class TestI extends TestIntf
{
    public
    TestI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    public List<C>
    opCArray(List<C> inSeq, CArrayHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List<C>
    opCList(List<C> inSeq, CListHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public C[]
    opCSeq(C[] inSeq, CSeqHolder outSeq, Ice.Current current)
    {
        seq = new ArrayList<C>(Arrays.asList(inSeq));
        outSeq.value = new C[seq.size()];
        seq.toArray(outSeq.value);
        return outSeq.value;
    }

    public List<Boolean>
    opBoolSeq(List<Boolean> inSeq, BoolSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List<Byte>
    opByteSeq(List<Byte> inSeq, ByteSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List<Map<Integer,String>>
    opDSeq(List<Map<Integer,String>> inSeq, DSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List<Double>
    opDoubleSeq(List<Double> inSeq, DoubleSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List<E>
    opESeq(List<E> inSeq, ESeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List<Float>
    opFloatSeq(List<Float> inSeq, FloatSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List<Integer>
    opIntSeq(List<Integer> inSeq, IntSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List<Long>
    opLongSeq(List<Long> inSeq, LongSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List<S>
    opSSeq(List<S> inSeq, SSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List<Short>
    opShortSeq(List<Short> inSeq, ShortSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List<String>
    opStringSeq(List<String> inSeq, StringSeqHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List<List<String>>
    opStringSeqSeq(List<List<String>> inSeq, StringSeqSeqHolder outSeq, Ice.Current current)
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
