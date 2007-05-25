// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        //
        // Install object factories.
        //
        Ice.ObjectFactory factory = new NodeFactory();
        communicator().addObjectFactory(factory, PersistentFile.ice_staticId());
        communicator().addObjectFactory(factory, PersistentDirectory.ice_staticId());

        //
        // Create an object adapter (stored in the _adapter
        // static member).
        //
        Ice.ObjectAdapter adapter =
            communicator().createObjectAdapterWithEndpoints("FreezeFilesystem", "default -p 10000");
        DirectoryI._adapter = adapter;
        FileI._adapter = adapter;

        //
        // Create the Freeze evictor (stored in the _evictor
        // static member).
        //
        Freeze.ServantInitializer init = new NodeInitializer();
        Freeze.Evictor evictor = Freeze.Util.createBackgroundSaveEvictor(adapter, _envName, "evictorfs", init, null, true);
        DirectoryI._evictor = evictor;
        FileI._evictor = evictor;

        adapter.addServantLocator(evictor, "");

        //
        // Create the root node if it doesn't exist.
        //
        Ice.Identity rootId = Ice.Util.stringToIdentity("RootDir");
        if(!evictor.hasObject(rootId))
        {
            PersistentDirectory root = new DirectoryI(rootId);
            root.nodeName = "/";
            root.nodes = new java.util.HashMap();
            evictor.add(root, rootId);
        }

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
