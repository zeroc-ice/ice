// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

public sealed class ThroughputI : _ThroughputDisp
{
    public ThroughputI()
    {
        _seq = new byte[seqSize.value];
    }
    
    public override byte[] echoByteSeq(byte[] seq, Ice.Current current)
    {
        return seq;
    }
    
    public override byte[] recvByteSeq(Ice.Current current)
    {
        return _seq;
    }
    
    public override void sendByteSeq(byte[] seq, Ice.Current current)
    {
    }
    
    private byte[] _seq;
}
