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

public sealed class CI : C_Disp
{
    public CI()
    {
    }
    
    public override string callA(Ice.Current current)
    {
        return "A";
    }
    
    public override string callC(Ice.Current current)
    {
        return "C";
    }
}
