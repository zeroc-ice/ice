// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

class LibraryServer extends Freeze.Application
{
    public int
    runFreeze(String[] args, Freeze.DBEnvironment dbEnv)
    {
	Ice.Properties properties = communicator().getProperties();
    
	Freeze.DB dbBooks = dbEnv.openDB("books", true);
	Freeze.DB dbAuthors = dbEnv.openDB("authors", true);
    
	//
	// Create an Evictor for books.
	//
	Freeze.PersistenceStrategy strategy;
	if(properties.getPropertyAsInt("Library.IdleStrategy") > 0)
	{
            strategy = dbBooks.createIdleStrategy();
	}
	else
	{
            strategy = dbBooks.createEvictionStrategy();
	}
	Freeze.Evictor evictor = dbBooks.createEvictor(strategy);
	int evictorSize = properties.getPropertyAsInt("Library.EvictorSize");
	if(evictorSize > 0)
	{
	    evictor.setSize(evictorSize);
	}
    
	//
	// Create an Object Adapter, use the Evictor as Servant
	// Locator.
	//
	Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Library");
	adapter.addServantLocator(evictor, "book");
    
	//
	// Create the library, and add it to the Object Adapter.
	//
	LibraryI library = new LibraryI(adapter, dbAuthors, evictor);
	adapter.add(library, Ice.Util.stringToIdentity("library"));
    
	//
	// Create and install a factory and initializer for books.
	//
	Ice.ObjectFactory bookFactory = new BookFactory(library);
	communicator().addObjectFactory(bookFactory, "::Book");
    
	//
	// Everything ok, let's go.
	//
	adapter.activate();

	shutdownOnInterrupt();
	communicator().waitForShutdown();
	defaultInterrupt();

	return 0;
    }

    LibraryServer(String dbEnvName)
    {
	super(dbEnvName);
    }
}

public class Server
{
    static public void
    main(String[] args)
    {
	LibraryServer app = new LibraryServer("db");
	app.main("demo.Freeze.library.Server", args, "config");
    }
}
