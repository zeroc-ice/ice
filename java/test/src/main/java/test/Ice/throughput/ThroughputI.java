// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.throughput;

import test.Ice.throughput.Demo.ByteSeqSize;
import test.Ice.throughput.Demo.Fixed;
import test.Ice.throughput.Demo.FixedSeqSize;
import test.Ice.throughput.Demo.StringDouble;
import test.Ice.throughput.Demo.StringDoubleSeqSize;
import test.Ice.throughput.Demo.StringSeqSize;
import test.Ice.throughput.Demo.Throughput;

public final class ThroughputI implements Throughput
{
    public ThroughputI()
    {
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

    @Override
    public boolean needsWarmup(com.zeroc.Ice.Current current)
    {
        _warmup = false;
        return _needsWarmup;
    }

    @Override
    public void startWarmup(com.zeroc.Ice.Current current)
    {
        _warmup = true;
    }

    @Override
    public void endWarmup(com.zeroc.Ice.Current current)
    {
        _warmup = false;
        _needsWarmup = false;
    }

    @Override
    public void sendByteSeq(byte[] seq, com.zeroc.Ice.Current current)
    {
    }

    @Override
    public byte[] recvByteSeq(com.zeroc.Ice.Current current)
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

    @Override
    public byte[] echoByteSeq(byte[] seq, com.zeroc.Ice.Current current)
    {
        return seq;
    }

    @Override
    public void sendStringSeq(String[] seq, com.zeroc.Ice.Current current)
    {
    }

    @Override
    public String[] recvStringSeq(com.zeroc.Ice.Current current)
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

    @Override
    public String[] echoStringSeq(String[] seq, com.zeroc.Ice.Current current)
    {
        return seq;
    }

    @Override
    public void sendStructSeq(StringDouble[] seq, com.zeroc.Ice.Current current)
    {
    }

    @Override
    public StringDouble[] recvStructSeq(com.zeroc.Ice.Current current)
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

    @Override
    public StringDouble[] echoStructSeq(StringDouble[] seq, com.zeroc.Ice.Current current)
    {
        return seq;
    }

    @Override
    public void sendFixedSeq(Fixed[] seq, com.zeroc.Ice.Current current)
    {
    }

    @Override
    public Fixed[] recvFixedSeq(com.zeroc.Ice.Current current)
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

    @Override
    public Fixed[] echoFixedSeq(Fixed[] seq, com.zeroc.Ice.Current current)
    {
        return seq;
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
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

    private boolean _needsWarmup = true;
    private boolean _warmup = false;
}
