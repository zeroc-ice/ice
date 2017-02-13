// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

public sealed class MyClassI : MyClassDisp_
{
    public override Task shutdownAsync(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        return null;
    }

    public override Task<MyClass_OpNVResult>
    opNVAsync(Dictionary<int, int> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpNVResult>(new MyClass_OpNVResult(i, i));
    }

    public override Task<MyClass_OpNRResult>
    opNRAsync(Dictionary<string, string> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpNRResult>(new MyClass_OpNRResult(i, i));
    }

    public override Task<MyClass_OpNDVResult>
    opNDVAsync(Dictionary<string, Dictionary<int, int>> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpNDVResult>(new MyClass_OpNDVResult(i, i));
    }

    public override Task<MyClass_OpNDRResult>
    opNDRAsync(Dictionary<string, Dictionary<string, string>> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpNDRResult>(new MyClass_OpNDRResult(i, i));
    }

    public override Task<MyClass_OpNDAISResult>
    opNDAISAsync(Dictionary<string, int[]> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpNDAISResult>(new MyClass_OpNDAISResult(i, i));
    }

    public override Task<MyClass_OpNDGISResult>
    opNDGISAsync(Dictionary<string, List<int>> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpNDGISResult>(new MyClass_OpNDGISResult(i, i));
    }

    public override Task<MyClass_OpNDASSResult>
    opNDASSAsync(Dictionary<string, string[]> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpNDASSResult>(new MyClass_OpNDASSResult(i, i));
    }

    public override Task<MyClass_OpNDGSSResult>
    opNDGSSAsync(Dictionary<string, List<string>> i, Ice.Current current)
    {
        return Task.FromResult<MyClass_OpNDGSSResult>(new MyClass_OpNDGSSResult(i, i));
    }
}
