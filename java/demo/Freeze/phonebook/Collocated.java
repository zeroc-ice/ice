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

class PhoneBookCollocated extends Freeze.Application
{
    public int
    runFreeze(String[] args, Freeze.DBEnvironment dbEnv)
    {
	Ice.Properties properties = communicator().getProperties();
    
	Freeze.DB dbPhoneBook = dbEnv.openDB("phonebook", true);
	Freeze.DB dbContacts = dbEnv.openDB("contacts", true);
    
	//
	// Create an Evictor for contacts.
	//
	Freeze.PersistenceStrategy strategy;
	if(properties.getPropertyAsInt("PhoneBook.IdleStrategy") > 0)
	{
            strategy = dbContacts.createIdleStrategy();
	}
	else
	{
            strategy = dbContacts.createEvictionStrategy();
	}
	Freeze.Evictor evictor = dbContacts.createEvictor(strategy);
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
	PhoneBookI phoneBook = new PhoneBookI(adapter, dbPhoneBook, evictor);
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

	return status;
    }

    PhoneBookCollocated(String dbEnvName)
    {
	super(dbEnvName);
    }
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
