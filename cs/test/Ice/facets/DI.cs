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

public sealed class DI : D_Disp
{
    public DI()
    {
    }
    
    public override string callA(Ice.Current current)
    {
        return "A";
    }
    
    public override string callB(Ice.Current current)
    {
        return "B";
    }
    
    public override string callC(Ice.Current current)
    {
        return "C";
    }
    
    public override string callD(Ice.Current current)
    {
        return "D";
    }
}
