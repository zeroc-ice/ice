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
