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
