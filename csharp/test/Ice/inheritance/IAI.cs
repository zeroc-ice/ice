// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public sealed class IAI : Test.MA.IADisp_
{
    public IAI()
    {
    }

    public override Test.MA.IAPrx iaop(Test.MA.IAPrx p, Ice.Current current)
    {
        return p;
    }
}
