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

class PhoneBookServer extends Ice.Application
{
    public int
    run(String[] args)
    {
	Ice.Properties properties = communicator().getProperties();

	//
	// Create the name index.
	//
	NameIndex index = new NameIndex("name");
	Freeze.Index[] indices = new Freeze.Index[1];
	indices[0] = index;

	//
	// Create an evictor for contacts.
	//
	Freeze.Evictor evictor = Freeze.Util.createEvictor(communicator(), _envName, "contacts", indices, true);
	int evictorSize = properties.getPropertyAsInt("PhoneBook.EvictorSize");
	if(evictorSize > 0)
	{
	    evictor.setSize(evictorSize);
	}
    
	//
	// Create an object adapter, use the evictor as servant
	// locator.
	//
	Ice.ObjectAdapter adapter = communicator().createObjectAdapter("PhoneBook");
	adapter.addServantLocator(evictor, "contact");
    
	//
	// Create the phonebook, and add it to the object adapter.
	//
	PhoneBookI phoneBook = new PhoneBookI(evictor, index);
	adapter.add(phoneBook, Ice.Util.stringToIdentity("phonebook"));
    
	//
	// Create and install a factory and initializer for contacts.
	//
	Ice.ObjectFactory contactFactory = new ContactFactory(evictor);
	communicator().addObjectFactory(contactFactory, "::Contact");
    
	//
	// Everything ok, let's go.
	//
	adapter.activate();

	shutdownOnInterrupt();
	communicator().waitForShutdown();
	defaultInterrupt();
	return 0;
    }

    PhoneBookServer(String envName)
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
	PhoneBookServer app = new PhoneBookServer("db");
	app.main("demo.Freeze.phonebook.Server", args);
    }
}
