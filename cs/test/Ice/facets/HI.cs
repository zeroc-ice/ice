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

public sealed class HI : H_Disp
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
