// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

public sealed class ThroughputI : Throughput_Disp
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
