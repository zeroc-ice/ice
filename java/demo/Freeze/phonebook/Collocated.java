// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
	Freeze.Evictor evictor;
	if(properties.getPropertyAsInt("PhoneBook.SaveAfterMutatingOperation") > 0)
	{
	    evictor = dbContacts.createEvictor(Freeze.EvictorPersistenceMode.SaveAfterMutatingOperation);
	}
	else
	{
	    evictor = dbContacts.createEvictor(Freeze.EvictorPersistenceMode.SaveUponEviction);
	}
	int evictorSize = properties.getPropertyAsInt("PhoneBook.EvictorSize");
	if(evictorSize > 0)
	{
	    evictor.setSize(evictorSize);
	}
    
	//
	// Create an Object Adapter, use the Evictor as Servant
	// Locator.
	//
	Ice.ObjectAdapter adapter = communicator().createObjectAdapter("PhoneBookAdapter");
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
	app.main("test.Freeze.phonebook.Collocated", args, "config");
    }
}
