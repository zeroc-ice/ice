// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Threading;
using System;

public class PriorityI : Test.PriorityDisp_
{

    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public override string getPriority(Ice.Current current)
    {
        return Thread.CurrentThread.Priority.ToString();
    }
}
