// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public sealed class BI : BDisp_
{
    public BI()
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
}
