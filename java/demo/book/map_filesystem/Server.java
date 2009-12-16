// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;

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
        // Install object factories
        //
        communicator().addObjectFactory(PersistentFile.ice_factory(), PersistentFile.ice_staticId());
        communicator().addObjectFactory(PersistentDirectory.ice_factory(), PersistentDirectory.ice_staticId());

        // Create an object adapter (stored in the _adapter static member)
        //
        Ice.ObjectAdapter adapter =
            communicator().createObjectAdapter("MapFilesystem");

        // Create a Freeze connection and the map
        //
        Freeze.Connection connection = Freeze.Util.createConnection(communicator(), _envName);
        IdentityNodeMap persistentMap = new IdentityNodeMap(connection, "mapfs", true); 

        // Set static members
        //
        FileI._map = persistentMap;
        DirectoryI._map = persistentMap;
        DirectoryI._adapter = adapter;

        // Find the persistent node for the root directory, or create it if not found
        //
        Ice.Identity rootId = Ice.Util.stringToIdentity("RootDir");
        PersistentDirectory pRoot = (PersistentDirectory)persistentMap.get(rootId);
        if(pRoot == null)
        {
            pRoot = new PersistentDirectory();
            pRoot.name = "/";
            pRoot.nodes = new java.util.HashMap<String, NodeDesc>();
            persistentMap.put(rootId, pRoot);
        }

        // Create the root directory (with name "/" and no parent)
        //
        DirectoryI root = new DirectoryI(rootId, pRoot, null);
        adapter.add(root, rootId);

        // Ready to accept requests now
        //
        adapter.activate();

        // Wait until we are done
        //
        communicator().waitForShutdown();

        // Clean up
        //
        connection.close();

        return 0;
    }

    public static void
    main(String[] args)
    {
        Server app = new Server("db");
        app.main("demo.book.map_filesystem.Server", args, "config.server");
        System.exit(0);
    }

    private String _envName;
}
