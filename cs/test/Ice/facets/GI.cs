// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


public sealed class GI : G_Disp
{
    public GI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }
    
    public override string callG(Ice.Current current)
    {
        return "G";
    }
    
    public override void shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }
    
    private Ice.Communicator _communicator;
}
