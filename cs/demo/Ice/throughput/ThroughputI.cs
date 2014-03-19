// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

public sealed class ThroughputI : ThroughputDisp_
{
    public ThroughputI()
    {
        _byteSeq = new byte[ByteSeqSize.value];
        _stringSeq = new string[StringSeqSize.value];
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
            _fixedSeq[i].i = 0;
            _fixedSeq[i].j = 0;
            _fixedSeq[i].d = 0;
        }
    }

    public override bool needsWarmup(Ice.Current current)
    {
        _warmup = false;
        return _needsWarmup;
    }

    public override void startWarmup(Ice.Current current)
    {
        _warmup = true;
    }

    public override void endWarmup(Ice.Current current)
    {
        _warmup = false;
        _needsWarmup = false;
    }

    public override void sendByteSeq(byte[] seq, Ice.Current current)
    {
    }

    public override byte[] recvByteSeq(Ice.Current current)
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

    public override byte[] echoByteSeq(byte[] seq, Ice.Current current)
    {
        return seq;
    }

    public override void sendStringSeq(string[] seq, Ice.Current current)
    {
    }

    public override string[] recvStringSeq(Ice.Current current)
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

    public override string[] echoStringSeq(string[] seq, Ice.Current current)
    {
        return seq;
    }

    public override void sendStructSeq(StringDouble[] seq, Ice.Current current)
    {
    }

    public override StringDouble[] recvStructSeq(Ice.Current current)
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

    public override StringDouble[] echoStructSeq(StringDouble[] seq, Ice.Current current)
    {
        return seq;
    }

    public override void sendFixedSeq(Fixed[] seq, Ice.Current current)
    {
    }

    public override Fixed[] recvFixedSeq(Ice.Current current)
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

    public override Fixed[] echoFixedSeq(Fixed[] seq, Ice.Current current)
    {
        return seq;
    }

    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private byte[] _byteSeq;
    private string[] _stringSeq;
    private StringDouble[] _structSeq;
    private Fixed[] _fixedSeq;

    private byte[] _emptyByteSeq = new byte[0];
    private string[] _emptyStringSeq = new string[0];
    private StringDouble[] _emptyStructSeq = new StringDouble[0];
    private Fixed[] _emptyFixedSeq = new Fixed[0];

    private bool _needsWarmup = true;
    private bool _warmup = false;
}
