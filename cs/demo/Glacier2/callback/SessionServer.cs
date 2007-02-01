// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class SessionServer : Ice.Application
{
    sealed class DummyPermissionVerifierI : Glacier2.PermissionsVerifierDisp_
    {
        public override bool checkPermissions(String userId, String password, out string reason, Ice.Current current)
        {
            reason = null;
            Console.WriteLine("verified user `" + userId + "' with password `" + password + "'");
            return true;
        }
    };

    public override int run(string[] args)
    {
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("SessionServer");
        adapter.add(new DummyPermissionVerifierI(), communicator().stringToIdentity("verifier"));
        adapter.add(new SessionManagerI(), communicator().stringToIdentity("sessionmanager"));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }

    public static void Main(string[] args)
    {
        SessionServer app = new SessionServer();
        int status = app.main(args, "config.sessionserver");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
