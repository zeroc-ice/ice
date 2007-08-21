// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using Test;

public sealed class MyClassI : Test.MyClass
{
    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public override Dictionary<int, int> opNV(Dictionary<int, int> i, out Dictionary<int, int> o,
                                              Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, string> opNR(Dictionary<string, string> i, out Dictionary<string, string> o,
                                                    Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, Dictionary<int, int>> opNDV(Dictionary<string, Dictionary<int, int>> i,
                                                                   out Dictionary<string, Dictionary<int, int>> o,
                                                                   Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, Dictionary<string, string>> opNDR(Dictionary<
                                                               string, Dictionary<string, string>> i,
                                                               out Dictionary<string, Dictionary<string, string>> o,
                                                               Ice.Current current)
    {
        o = i;
        return i;
    }

    public override OV opOV(OV i, out OV o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override OR opOR(OR i, out OR o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override ODV opODV(ODV i, out ODV o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override ODR opODR(ODR i, out ODR o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, ODV> opNODV(Dictionary<string, ODV> i, out Dictionary<string, ODV> o,
                                                   Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, ODR> opNODR(Dictionary<string, ODR> i, out Dictionary<string, ODR> o,
                                                   Ice.Current current)
    {
        o = i;
        return i;
    }

    public override ONDV opONDV(ONDV i, out ONDV o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override ONDR opONDR(ONDR i, out ONDR o, Ice.Current current)
    {
        o = i;
        return i;
    }
}
