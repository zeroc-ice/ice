// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class PhoneBookServer extends Ice.Application
{
    public int
    run(String[] args)
    {
	Ice.Properties properties = communicator().getProperties();

	//
	// Create and install a factory for contacts.
	//
	ContactFactory contactFactory = new ContactFactory();
	communicator().addObjectFactory(contactFactory, "::Demo::Contact");

	//
	// Create an object adapter
	//
	Ice.ObjectAdapter adapter = communicator().createObjectAdapter("PhoneBook");

	//
	// Create the name index.
	//
	NameIndex index = new NameIndex("name");
	Freeze.Index[] indices = new Freeze.Index[1];
	indices[0] = index;

	//
	// Create an evictor for contacts.
	//
	Freeze.Evictor evictor = Freeze.Util.createEvictor(adapter, _envName, "contacts", null, indices, true);
	int evictorSize = properties.getPropertyAsInt("PhoneBook.EvictorSize");
	if(evictorSize > 0)
	{
	    evictor.setSize(evictorSize);
	}
    
	//
	// Set the evictor in the contact factory
	//
	contactFactory.setEvictor(evictor);

	//
	// Register the evictor with the adapter
	//
	adapter.addServantLocator(evictor, "contact");
    
	//
	// Create the phonebook, and add it to the object adapter.
	//
	PhoneBookI phoneBook = new PhoneBookI(evictor, contactFactory, index);
	adapter.add(phoneBook, Ice.Util.stringToIdentity("phonebook"));
        
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
	app.main("demo.Freeze.phonebook.Server", args, "config");
    }
}
