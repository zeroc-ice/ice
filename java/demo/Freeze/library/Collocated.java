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

class LibraryCollocated extends Ice.Application
{
    public int
    run(String[] args)
    {
	Ice.Properties properties = communicator().getProperties();

	//
	// Create an Object Adapter
	//
	Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Library");

	//
	// Create an Evictor for books.
	//
	Freeze.Evictor evictor = Freeze.Util.createEvictor(adapter, _envName, "books", null, null, true);
	int evictorSize = properties.getPropertyAsInt("Library.EvictorSize");
	if(evictorSize > 0)
	{
	    evictor.setSize(evictorSize);
	}

	adapter.addServantLocator(evictor, "book");
    
	//
	// Create the library, and add it to the Object Adapter.
	//
	LibraryI library = new LibraryI(communicator(), _envName, "authors", evictor);
	adapter.add(library, Ice.Util.stringToIdentity("library"));
    
	//
	// Create and install a factory and initializer for books.
	//
	Ice.ObjectFactory bookFactory = new BookFactory(library);
	communicator().addObjectFactory(bookFactory, "::Book");
    
	//
	// Everything ok, let's go.
	//
	int status = RunParser.runParser(appName(), args, communicator());
	adapter.deactivate();
	adapter.waitForDeactivate();

	library.close();
	return status;
    }

    LibraryCollocated(String envName)
    {
	_envName = envName;
    }

    private String _envName;
}

public class Collocated
{
    static public void
    main(String[] args)
    {
	LibraryCollocated app = new LibraryCollocated("db");
	app.main("demo.Freeze.library.Collocated", args, "config");
    }
}
