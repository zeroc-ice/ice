// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public sealed class IB2I : Test.MB.IB2Disp_
{
    public IB2I()
    {
    }

    public override Test.MA.IAPrx iaop(Test.MA.IAPrx p, Ice.Current current)
    {
        return p;
    }

    public override Test.MB.IB2Prx ib2op(Test.MB.IB2Prx p, Ice.Current current)
    {
        return p;
    }
}
