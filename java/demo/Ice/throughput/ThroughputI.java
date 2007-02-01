// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public final class ThroughputI extends _ThroughputDisp
{
    public
    ThroughputI()
    {
        _warmup = true; 

        _byteSeq = new byte[ByteSeqSize.value];

        _stringSeq = new String[StringSeqSize.value];
        for(int i = 0; i < StringSeqSize.value; ++i)
        {
            _stringSeq[i] = "hello";
        }

        _structSeq = new StringDouble[StringDoubleSeqSize.value];
        for(int i = 0; i < StringDoubleSeqSize.value; ++i)
        {
            _structSeq[i] = new StringDouble();
            _structSeq[i].s = "hello";
            _structSeq[i].d = 3.14;
        }

        _fixedSeq = new Fixed[FixedSeqSize.value];
        for(int i = 0; i < FixedSeqSize.value; ++i)
        {
            _fixedSeq[i] = new Fixed();
            _fixedSeq[i].i = 0;
            _fixedSeq[i].j = 0;
            _fixedSeq[i].d = 0;
        }
    }

    public void
    endWarmup(Ice.Current current)
    {
        _warmup = false;
    }

    public void
    sendByteSeq(byte[] seq, Ice.Current current)
    {
    }

    public byte[]
    recvByteSeq(Ice.Current current)
    {
        if(_warmup)
        {
            return _emptyByteSeq;
        }
        else
        {
            return _byteSeq;
        }
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
        if(_warmup)
        {
            return _emptyStringSeq;
        }
        else
        {
            return _stringSeq;
        }
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
        if(_warmup)
        {
            return _emptyStructSeq;
        }
        else
        {
            return _structSeq;
        }
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
        if(_warmup)
        {
            return _emptyFixedSeq;
        }
        else
        {
            return _fixedSeq;
        }
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

    private byte[] _emptyByteSeq = new byte[0];
    private String[] _emptyStringSeq = new String[0];
    private StringDouble[] _emptyStructSeq = new StringDouble[0];
    private Fixed[] _emptyFixedSeq = new Fixed[0];

    private boolean _warmup;
}
