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

public sealed class FI : F_Disp
{
    public FI()
    {
    }
    
    public override string callE(Ice.Current current)
    {
        return "E";
    }
    
    public override string callF(Ice.Current current)
    {
        return "F";
    }
}
