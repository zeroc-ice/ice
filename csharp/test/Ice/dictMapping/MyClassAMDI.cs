// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

    public override void opNDAIS_async(AMD_MyClass_opNDAIS cb, Dictionary<string, int[]> i, Ice.Current current)
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

    public override void opNDGSS_async(AMD_MyClass_opNDGSS cb, Dictionary<string, List<string>> i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }
}
