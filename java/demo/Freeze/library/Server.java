// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
	Freeze.Evictor evictor;
	if(properties.getPropertyAsInt("Library.SaveAfterMutatingOperation") > 0)
	{
	    evictor = dbBooks.createEvictor(Freeze.EvictorPersistenceMode.SaveAfterMutatingOperation);
	}
	else
	{
	    evictor = dbBooks.createEvictor(Freeze.EvictorPersistenceMode.SaveUponEviction);
	}
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
	ignoreInterrupt();

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
	app.main("test.Freeze.library.Server", args, "config");
    }
}
