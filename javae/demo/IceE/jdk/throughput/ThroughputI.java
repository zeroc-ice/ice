// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public final class ThroughputI extends _ThroughputDisp
{
    public
    ThroughputI(int reduce)
    {
        _byteSeq = new byte[ByteSeqSize.value / reduce];

	_stringSeq = new String[StringSeqSize.value / reduce];
	for(int i = 0; i < StringSeqSize.value / reduce; ++i)
	{
	    _stringSeq[i] = "hello";
	}

	_structSeq = new StringDouble[StringDoubleSeqSize.value / reduce];
	for(int i = 0; i < StringDoubleSeqSize.value / reduce; ++i)
	{
	    _structSeq[i] = new StringDouble();
	    _structSeq[i].s = "hello";
	    _structSeq[i].d = 3.14;
	}

	_fixedSeq = new Fixed[FixedSeqSize.value / reduce];
	for(int i = 0; i < FixedSeqSize.value / reduce; ++i)
	{
	    _fixedSeq[i] = new Fixed();
	    _fixedSeq[i].i = 0;
	    _fixedSeq[i].j = 0;
	    _fixedSeq[i].d = 0;
	}
    }

    public void
    sendByteSeq(byte[] seq, Ice.Current current)
    {
    }

    public byte[]
    recvByteSeq(Ice.Current current)
    {
        return _byteSeq;
    }

    public byte[]
    echoByteSeq(byte[] seq, Ice.Current current)
    {
        return seq;
    }

    public void
    sendStringSeq(String[] seq, Ice.Current current)
    {
    }

    public String[]
    recvStringSeq(Ice.Current current)
    {
        return _stringSeq;
    }

    public String[]
    echoStringSeq(String[] seq, Ice.Current current)
    {
        return seq;
    }

    public void
    sendStructSeq(StringDouble[] seq, Ice.Current current)
    {
    }

    public StringDouble[]
    recvStructSeq(Ice.Current current)
    {
        return _structSeq;
    }

    public StringDouble[]
    echoStructSeq(StringDouble[] seq, Ice.Current current)
    {
        return seq;
    }

    public void
    sendFixedSeq(Fixed[] seq, Ice.Current current)
    {
    }

    public Fixed[]
    recvFixedSeq(Ice.Current current)
    {
        return _fixedSeq;
    }

    public Fixed[]
    echoFixedSeq(Fixed[] seq, Ice.Current current)
    {
        return seq;
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private byte[] _byteSeq;
    private String[] _stringSeq;
    private StringDouble[] _structSeq;
    private Fixed[] _fixedSeq;
}
