// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

class PhoneBookServer extends Freeze.Application
{
    PhoneBookServer(String dbEnvName)
    {
	super(dbEnvName);
    }

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
	int v = 0;
        try
        {
            v = Integer.parseInt(properties.getProperty("PhoneBook.SaveAfterMutatingOperation"));
        }
        catch (NumberFormatException ex)
        {
	}

	if(v != 0)
	{
	    evictor = dbContacts.createEvictor(Freeze.EvictorPersistenceMode.SaveAfterMutatingOperation);
	}
	else
	{
	    evictor = dbContacts.createEvictor(Freeze.EvictorPersistenceMode.SaveUponEviction);
	}

        try
        {
            v = Integer.parseInt(properties.getProperty("PhoneBook.EvictorSize"));
        }
        catch (NumberFormatException ex)
        {
        }

	if(v != 0)
	{
	    evictor.setSize(v);
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
	adapter.activate();

	//
	// TODO:
	//
	//shutdownOnInterrupt();

	communicator().waitForShutdown();

	//ignoreInterrupt();

	return 0;
    }
}

public class Server
{
    static public void
    main(String[] args)
    {
	PhoneBookServer app = new PhoneBookServer("db");
	app.main("test.Freeze.phonebook.Server", args, "config");
    }
}
