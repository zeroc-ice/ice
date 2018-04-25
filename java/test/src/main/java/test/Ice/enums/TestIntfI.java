// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.enums;

import test.Ice.enums.Test.*;

public final class TestIntfI implements TestIntf
{
    @Override
    public TestIntf.OpByteResult opByte(ByteEnum b1, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpByteResult(b1, b1);
    }

    @Override
    public TestIntf.OpShortResult opShort(ShortEnum s1, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpShortResult(s1, s1);
    }

    @Override
    public TestIntf.OpIntResult opInt(IntEnum i1, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpIntResult(i1, i1);
    }

    @Override
    public TestIntf.OpSimpleResult opSimple(SimpleEnum s1, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpSimpleResult(s1, s1);
    }

    @Override
    public TestIntf.OpByteSeqResult opByteSeq(ByteEnum[] b1, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpByteSeqResult(b1, b1);
    }

    @Override
    public TestIntf.OpShortSeqResult opShortSeq(ShortEnum[] s1, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpShortSeqResult(s1, s1);
    }

    @Override
    public TestIntf.OpIntSeqResult opIntSeq(IntEnum[] i1, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpIntSeqResult(i1, i1);
    }

    @Override
    public TestIntf.OpSimpleSeqResult opSimpleSeq(SimpleEnum[] s1, com.zeroc.Ice.Current current)
    {
        return new TestIntf.OpSimpleSeqResult(s1, s1);
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
