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

public final class ThroughputI extends _ThroughputDisp
{
    public
    ThroughputI()
    {
        _seq = new byte[seqSize.value];
    }

    public byte[]
    echoByteSeq(byte[] seq, Ice.Current current)
    {
        return seq;
    }

    public byte[]
    recvByteSeq(Ice.Current current)
    {
        return _seq;
    }

    public void
    sendByteSeq(byte[] seq, Ice.Current current)
    {
    }

    private byte[] _seq;
}
