// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public sealed class CBI : Test.MB.CB
{
    public CBI()
    {
    }
    
    public override Test.MA.CAPrx caop(Test.MA.CAPrx p, Ice.Current current)
    {
        return p;
    }
    
    public override Test.MB.CBPrx cbop(Test.MB.CBPrx p, Ice.Current current)
    {
        return p;
    }
}
