// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : TestCommon.Application
{
    public override int run(string[] args)
    {
        GPrx g = AllTests.allTests(this);
        g.shutdown();
        return 0;
    }

    public static int Main(string[] args)
    {
        Client app = new Client();
        return app.runmain(args);
    }
}
