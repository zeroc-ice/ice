// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class CBI extends Test.MB.CB
{
    public
    CBI()
    {
    }

    public Test.MA.CAPrx
    caop(Test.MA.CAPrx p, Ice.Current current)
    {
        return p;
    }

    public Test.MB.CBPrx
    cbop(Test.MB.CBPrx p, Ice.Current current)
    {
        return p;
    }
}
