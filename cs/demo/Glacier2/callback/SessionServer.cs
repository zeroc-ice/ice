// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("Glacier2CallbackSessionServer")]
[assembly: AssemblyDescription("Glacier2 callback demo session server")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class SessionServer
{
    public class App : Ice.Application
    {
        sealed class DummyPermissionVerifierI : Glacier2.PermissionsVerifierDisp_
        {
            public override bool checkPermissions(String userId, String password, out string reason,
                                                  Ice.Current current)
            {
                reason = null;
                Console.WriteLine("verified user `" + userId + "' with password `" + password + "'");
                return true;
            }
        }

        public override int run(string[] args)
        {
            if(args.Length > 0)
            {
                Console.Error.WriteLine(appName() + ": too many arguments");
                return 1;
            }

            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("SessionServer");
            adapter.add(new DummyPermissionVerifierI(), communicator().stringToIdentity("verifier"));
            adapter.add(new SessionManagerI(), communicator().stringToIdentity("sessionmanager"));
            adapter.activate();
            communicator().waitForShutdown();
            return 0;
        }
    }

    public static void Main(string[] args)
    {
        App app = new App();
        int status = app.main(args, "config.sessionserver");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
