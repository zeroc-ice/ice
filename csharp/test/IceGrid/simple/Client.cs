// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
        bool withDeploy = false;
        for(int i = 0; i < args.Length; i++)
        {
            if(args[i].Equals("--with-deploy"))
            {
                withDeploy = true;
                break;
            }
        }

        if(!withDeploy)
        {
            AllTests.allTests(this);
        }
        else
        {
            AllTests.allTestsWithDeploy(this);
        }

        return 0;
    }

    public static int Main(string[] args)
    {
        Client app = new Client();
        return app.runmain(args);
    }
}
