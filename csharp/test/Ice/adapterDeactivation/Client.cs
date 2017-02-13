// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System;
using System.Diagnostics;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    internal class App : Ice.Application
    {
        public override int run(string[] args)
        {
            AllTests.allTests(communicator());
            return 0;
        }
    }
    
    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args);
    }
}
