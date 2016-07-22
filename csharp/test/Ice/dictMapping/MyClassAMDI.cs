// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;
using System;
using Test;

public sealed class MyClassI : MyClass
{
    public override void shutdownAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        response();
    }

    public override void
    opNVAsync(Dictionary<int, int> i, Action<MyClass_OpNVResult> response, Action<Exception> exception,
              Ice.Current current)
    {
        response(new MyClass_OpNVResult(i, i));
    }

    public override void
    opNRAsync(Dictionary<string, string> i, Action<MyClass_OpNRResult> response, Action<Exception> exception,
              Ice.Current current)
    {
        response(new MyClass_OpNRResult(i, i));
    }

    public override void
    opNDVAsync(Dictionary<string, Dictionary<int, int>> i, Action<MyClass_OpNDVResult> response,
               Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpNDVResult(i, i));
    }

    public override void
    opNDRAsync(Dictionary<string, Dictionary<string, string>> i, Action<MyClass_OpNDRResult> response,
               Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpNDRResult(i, i));
    }

    public override void
    opNDAISAsync(Dictionary<string, int[]> i, Action<MyClass_OpNDAISResult> response,
                 Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpNDAISResult(i, i));
    }

    public override void
    opNDGISAsync(Dictionary<string, List<int>> i, Action<MyClass_OpNDGISResult> response, Action<Exception> exception,
                 Ice.Current current)
    {
        response(new MyClass_OpNDGISResult(i, i));
    }

    public override void
    opNDASSAsync(Dictionary<string, string[]> i, Action<MyClass_OpNDASSResult> response, Action<Exception> exception,
                 Ice.Current current)
    {
        response(new MyClass_OpNDASSResult(i, i));
    }

    public override void
    opNDGSSAsync(Dictionary<string, List<string>> i, Action<MyClass_OpNDGSSResult> response,
                 Action<Exception> exception, Ice.Current current)
    {
        response(new MyClass_OpNDGSSResult(i, i));
    }
}
