// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;
import java.util.*;

public final class TestI extends TestIntf
{
    public
    TestI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    public List<Test.C>
    opCArray(List<Test.C> inSeq, CArrayHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public List<Test.C>
    opCList(List<Test.C> inSeq, CListHolder outSeq, Ice.Current current)
    {
        outSeq.value = inSeq;
        return inSeq;
    }

    public C[]
    opCSeq(C[] inSeq, CSeqHolder outSeq, Ice.Current current)
    {
        seq = new ArrayList<Test.C>(Arrays.asList(inSeq));
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

    public List<Test.E>
    opESeq(List<Test.E> inSeq, ESeqHolder outSeq, Ice.Current current)
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

    public List<Test.S>
    opSSeq(List<Test.S> inSeq, SSeqHolder outSeq, Ice.Current current)
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
