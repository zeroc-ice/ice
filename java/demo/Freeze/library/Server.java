// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class LibraryServer extends Ice.Application
{
    public int
    run(String[] args)
    {
        Ice.Properties properties = communicator().getProperties();
    
        //
        // Create an object adapter
        //
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Library");

        //
        // Create an evictor for books.
        //
        Freeze.Evictor evictor = Freeze.Util.createBackgroundSaveEvictor(adapter, _envName, "books", null, null, true);
        int evictorSize = properties.getPropertyAsInt("EvictorSize");
        if(evictorSize > 0)
        {
            evictor.setSize(evictorSize);
        }
    
        adapter.addServantLocator(evictor, "book");
    
        //
        // Create the library, and add it to the object adapter.
        //
        LibraryI library = new LibraryI(communicator(), _envName, "authors", evictor);
        adapter.add(library, communicator().stringToIdentity("library"));
    
        //
        // Create and install a factory for books.
        //
        Ice.ObjectFactory bookFactory = new BookFactory(library);
        communicator().addObjectFactory(bookFactory, "::Demo::Book");
    
        //
        // Everything ok, let's go.
        //
        adapter.activate();

        shutdownOnInterrupt();
        communicator().waitForShutdown();
        defaultInterrupt();

        library.close();
        return 0;
    }

    LibraryServer(String envName)
    {
        _envName = envName;
    }

    private String _envName;
}

public class Server
{
    static public void
    main(String[] args)
    {
        LibraryServer app = new LibraryServer("db");
        app.main("demo.Freeze.library.Server", args, "config.server");
    }
}
