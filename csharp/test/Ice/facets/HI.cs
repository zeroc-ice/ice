// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public sealed class HI : HDisp_
{
    public HI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    public override string callG(Ice.Current current)
    {
        return "G";
    }

    public override string callH(Ice.Current current)
    {
        return "H";
    }

    public override void shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    private Ice.Communicator _communicator;
}
