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
