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
	Freeze.Evictor evictor = Freeze.Util.createEvictor(adapter, _envName, "books", null, null, true);
	int evictorSize = properties.getPropertyAsInt("Library.EvictorSize");
	if(evictorSize > 0)
	{
	    evictor.setSize(evictorSize);
	}
    
	adapter.addServantLocator(evictor, "book");
    
	//
	// Create the library, and add it to the object adapter.
	//
	LibraryI library = new LibraryI(communicator(), _envName, "authors", evictor);
	adapter.add(library, Ice.Util.stringToIdentity("library"));
    
	//
	// Create and install a factory for books.
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
	app.main("demo.Freeze.library.Server", args, "config");
    }
}
