// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;
import FilesystemI.*;

public class Server extends Ice.Application
{
    public
    Server(String envName)
    {
        _envName = envName;
    }

    public int
    run(String[] args)
    {
        //
        // Terminate cleanly on receipt of a signal.
        //
        shutdownOnInterrupt();

        //
        // Install object factories.
        //
        communicator().addObjectFactory(PersistentFile.ice_factory(), PersistentFile.ice_staticId());
        communicator().addObjectFactory(PersistentDirectory.ice_factory(), PersistentDirectory.ice_staticId());

        //
        // Create an object adapter.
        //
        Ice.ObjectAdapter adapter =
            communicator().createObjectAdapterWithEndpoints("MapFilesystem", "default -p 10000");

        DirectoryI._envName = _envName;
        DirectoryI._dbName = "mapfs";
        FileI._envName = _envName;
        FileI._dbName = "mapfs";

        //
        // Find the persistent node for the root directory,
        // or create it if not found.
        //
        Freeze.Connection connection = Freeze.Util.createConnection(communicator(), _envName);
        IdentityNodeMap persistentMap = new IdentityNodeMap(connection, FileI._dbName, true);

        Ice.Identity rootId = communicator().stringToIdentity("RootDir");
        PersistentDirectory pRoot = (PersistentDirectory)persistentMap.get(rootId);
        if(pRoot == null)
        {
            pRoot = new PersistentDirectory();
            pRoot.name ="/";
            pRoot.nodes = new java.util.HashMap<java.lang.String, NodeDesc>();
            persistentMap.put(rootId, pRoot);
        }
        DirectoryI dir = new DirectoryI(adapter, rootId, pRoot, null);
        dir.activate(adapter);

        //
        // Ready to accept requests now.
        //
        adapter.activate();

        //
        // Wait until we are done.
        //
        communicator().waitForShutdown();

        return 0;
    }

    public static void
    main(String[] args)
    {
        Server app = new Server("db");
        int status = app.main("Server", args, "config.server");
        System.exit(status);
    }

    private String _envName;
}
