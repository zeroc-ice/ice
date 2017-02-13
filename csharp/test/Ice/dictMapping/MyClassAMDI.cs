// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using Test;

public sealed class MyClassI : MyClass
{
    public override void shutdown_async(AMD_MyClass_shutdown cb, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }

    public override void opNV_async(AMD_MyClass_opNV cb, Dictionary<int, int> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opNR_async(AMD_MyClass_opNR cb, Dictionary<string, string> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opNDV_async(AMD_MyClass_opNDV cb, Dictionary<string, Dictionary<int, int>> i,
                                     Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opNDR_async(AMD_MyClass_opNDR cb, Dictionary<string, Dictionary<string, string>> i,
                                     Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opOV_async(AMD_MyClass_opOV cb, OV i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opOR_async(AMD_MyClass_opOR cb, OR i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opODV_async(AMD_MyClass_opODV cb, ODV i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opODR_async(AMD_MyClass_opODR cb, ODR i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opNODV_async(AMD_MyClass_opNODV cb, Dictionary<string, ODV> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opNODR_async(AMD_MyClass_opNODR cb, Dictionary<string, ODR> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opONDV_async(AMD_MyClass_opONDV cb, ONDV i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opONDR_async(AMD_MyClass_opONDR cb, ONDR i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opNDAIS_async(AMD_MyClass_opNDAIS cb, Dictionary<string, int[]> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opNDCIS_async(AMD_MyClass_opNDCIS cb, Dictionary<string, CIS> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opNDGIS_async(AMD_MyClass_opNDGIS cb, Dictionary<string, List<int>> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opNDASS_async(AMD_MyClass_opNDASS cb, Dictionary<string, string[]> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opNDCSS_async(AMD_MyClass_opNDCSS cb, Dictionary<string, CSS> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opNDGSS_async(AMD_MyClass_opNDGSS cb, Dictionary<string, List<string>> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opODAIS_async(AMD_MyClass_opODAIS cb, ODAIS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opODCIS_async(AMD_MyClass_opODCIS cb, ODCIS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opODGIS_async(AMD_MyClass_opODGIS cb, ODGIS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opODASS_async(AMD_MyClass_opODASS cb, ODASS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opODCSS_async(AMD_MyClass_opODCSS cb, ODCSS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    public override void opODGSS_async(AMD_MyClass_opODGSS cb, ODGSS i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }
}
