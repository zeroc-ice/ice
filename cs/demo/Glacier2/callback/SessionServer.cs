// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class SessionServer
{
    sealed class DummyPermissionVerifierI : Glacier2._PermissionsVerifierDisp
    {
	public override bool checkPermissions(String userId, String password, out string reason, Ice.Current current)
	{
            reason = null;
	    Console.WriteLine("verified user `" + userId + "' with password `" + password + "'");
	    return true;
	}
    };

    class Application : Ice.Application
    {
	public override int run(string[] args)
	{
	    Ice.ObjectAdapter adapter = communicator().createObjectAdapter("SessionServer");
	    adapter.add(new DummyPermissionVerifierI(), Ice.Util.stringToIdentity("verifier"));
	    adapter.add(new SessionManagerI(), Ice.Util.stringToIdentity("sessionmanager"));
	    adapter.activate();
	    communicator().waitForShutdown();
	    return 0;
	}
    };

    public static void Main(string[] args)
    {
	Application app = new Application();
        int status = app.main(args, "config.sessionserver");
        Environment.Exit(status);
    }
}
