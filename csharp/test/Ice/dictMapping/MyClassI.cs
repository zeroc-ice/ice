// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

    public override Dictionary<string, int[]> opNDAIS(Dictionary<string, int[]> i,
                                                      out Dictionary<string, int[]> o,
                                                      Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, CIS> opNDCIS(Dictionary<string, CIS> i,
                                                    out Dictionary<string, CIS> o,
                                                    Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, List<int>> opNDGIS(Dictionary<string, List<int>> i,
                                                          out Dictionary<string, List<int>> o,
                                                          Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, string[]> opNDASS(Dictionary<string, string[]> i,
                                                         out Dictionary<string, string[]> o,
                                                         Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, CSS> opNDCSS(Dictionary<string, CSS> i,
                                                    out Dictionary<string, CSS> o,
                                                    Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, List<string>> opNDGSS(Dictionary<string, List<string>> i,
                                                             out Dictionary<string, List<string>> o,
                                                             Ice.Current current)
    {
        o = i;
        return i;
    }

    public override ODAIS opODAIS(ODAIS i, out ODAIS o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override ODCIS opODCIS(ODCIS i, out ODCIS o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override ODGIS opODGIS(ODGIS i, out ODGIS o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override ODASS opODASS(ODASS i, out ODASS o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override ODCSS opODCSS(ODCSS i, out ODCSS o, Ice.Current current)
    {
        o = i;
        return i;
    }

    public override ODGSS opODGSS(ODGSS i, out ODGSS o, Ice.Current current)
    {
        o = i;
        return i;
    }
}
