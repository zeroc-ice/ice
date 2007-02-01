// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class SessionServer
{
    static final class DummyPermissionVerifierI extends Glacier2._PermissionsVerifierDisp
    {
        public boolean
        checkPermissions(String userId, String password, Ice.StringHolder reason, Ice.Current current)
        {
            System.out.println("verified user `" + userId + "' with password `" + password + "'");
            return true;
        }
    };

    static class Application extends Ice.Application
    {
        public int
        run(String[] args)
        {
            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("SessionServer");
            adapter.add(new DummyPermissionVerifierI(), communicator().stringToIdentity("verifier"));
            adapter.add(new SessionManagerI(), communicator().stringToIdentity("sessionmanager"));
            adapter.activate();
            communicator().waitForShutdown();
            return 0;
        }
    };

    public static void
    main(String[] args)
    {
        Application app = new Application();
        int status = app.main("Server", args, "config.sessionserver");
        System.exit(status);
    }
}
