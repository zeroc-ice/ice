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

class PhoneBookCollocated extends Ice.Application
{
    public int
    run(String[] args)
    {
	Ice.Properties properties = communicator().getProperties();
    
	//
	// Create and install a factory and initializer for contacts.
	//
	ContactFactory contactFactory = new ContactFactory();
	communicator().addObjectFactory(contactFactory, "::Contact");

	//
	// Create the Name index
	//
	NameIndex index = new NameIndex("name");
	Freeze.Index[] indices = new Freeze.Index[1];
	indices[0] = index;

	//
	// Create an Evictor for contacts.
	//
	Freeze.Evictor evictor = Freeze.Util.createEvictor(communicator(), _envName, "contacts", indices, true);
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
	// Create an Object Adapter, use the Evictor as Servant
	// Locator.
	//
	Ice.ObjectAdapter adapter = communicator().createObjectAdapter("PhoneBook");
	adapter.addServantLocator(evictor, "contact");
    
	//
	// Create the phonebook, and add it to the Object Adapter.
	//
	PhoneBookI phoneBook = new PhoneBookI(evictor, contactFactory, index);
	adapter.add(phoneBook, Ice.Util.stringToIdentity("phonebook"));
    
	//
	// Everything ok, let's go.
	//
	int status = RunParser.runParser(appName(), args, communicator());
	adapter.deactivate();
	adapter.waitForDeactivate();

	return status;
    }

    PhoneBookCollocated(String envName)
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
	PhoneBookCollocated app = new PhoneBookCollocated("db");
	app.main("demo.Freeze.phonebook.Collocated", args, "config");
    }
}
