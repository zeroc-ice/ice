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
	// Create an Evictor for contacts.
	//
	Freeze.Evictor evictor = Freeze.Util.createEvictor(communicator(), _envName, "contacts", true);
	int evictorSize = properties.getPropertyAsInt("PhoneBook.EvictorSize");
	if(evictorSize > 0)
	{
	    evictor.setSize(evictorSize);
	}
    
	//
	// Create an Object Adapter, use the Evictor as Servant
	// Locator.
	//
	Ice.ObjectAdapter adapter = communicator().createObjectAdapter("PhoneBook");
	adapter.addServantLocator(evictor, "contact");
    
	//
	// Create the phonebook, and add it to the Object Adapter.
	//
	PhoneBookI phoneBook = new PhoneBookI(communicator(), _envName, "phonebook", evictor);
	adapter.add(phoneBook, Ice.Util.stringToIdentity("phonebook"));
    
	//
	// Create and install a factory and initializer for contacts.
	//
	Ice.ObjectFactory contactFactory = new ContactFactory(phoneBook, evictor);
	Freeze.ServantInitializer contactInitializer = (Freeze.ServantInitializer)contactFactory;
	communicator().addObjectFactory(contactFactory, "::Contact");
	evictor.installServantInitializer(contactInitializer);
    
	//
	// Everything ok, let's go.
	//
	int status = RunParser.runParser(appName(), args, communicator());
	adapter.deactivate();
	adapter.waitForDeactivate();
	phoneBook.close();

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
