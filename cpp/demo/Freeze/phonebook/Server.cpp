// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Application.h>
#include <ServantFactory.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

class PhoneBookServer : public Application
{
    int run(int argc, char* argv[]);
};

int
PhoneBookServer::run(int argc, char* argv[])
{
    cout << "starting up..." << endl;
    ignoreInterrupt();
    
    int status;
    DBEnvironmentPtr dbEnv;
    
    try
    {
	PropertiesPtr properties = communicator()->getProperties();
	string value;

        //
	// Open the database environment and the phonebook and
	// contacts database within this environment.
	//
	dbEnv = Freeze::initialize(communicator(), "db");
	DBPtr dbPhoneBook = dbEnv->openDB("phonebook");
	DBPtr dbContacts = dbEnv->openDB("contacts");
	
	//
	// Create an Evictor for contacts.
	//
	EvictorPtr evictor;
	value = properties->getProperty("PhoneBook.SaveAfterMutatingOperation");
	if(!value.empty() && atoi(value.c_str()) > 0)
	{
	    evictor = dbContacts->createEvictor(SaveAfterMutatingOperation);
	}
	else
	{
	    evictor = dbContacts->createEvictor(SaveUponEviction);
	}
	value = properties->getProperty("PhoneBook.EvictorSize");
	if(!value.empty())
	{
	    evictor->setSize(atoi(value.c_str()));
	}
	
	//
	// Create an Object Adapter, use the Evictor as Servant Locator.
	//
	ObjectAdapterPtr adapter = communicator()->createObjectAdapter("PhoneBookAdapter");
	adapter->setServantLocator(evictor);
	
	//
	// Create the phonebook, and add it to the Object Adapter.
	//
	PhoneBookIPtr phoneBook = new PhoneBookI(adapter, dbPhoneBook, evictor);
	adapter->add(phoneBook, "phonebook");
	
	//
	// Create and install a factory and initializer for contacts.
	//
	ServantFactoryPtr contactFactory = new ContactFactory(phoneBook, evictor);
	ServantInitializerPtr contactInitializer = ServantInitializerPtr::dynamicCast(contactFactory);
	communicator()->installServantFactory(contactFactory, "::Contact");
	evictor->installServantInitializer(contactInitializer);
	
	//
	// Everything ok, let's go.
	//
	adapter->activate();
	shutdownOnInterrupt();
	communicator()->waitForShutdown();
	ignoreInterrupt();
	status = EXIT_SUCCESS;
    }
    catch(const LocalException& ex)
    {
	cerr << argv[0] << ": local exception: " << ex << endl;
	status = EXIT_FAILURE;
    }
    catch(const DBExceptionPtrE& ex)
    {
	cerr << argv[0] << ": database exception: " << ex->message << endl;
	status = EXIT_FAILURE;
    }
    catch(...)
    {
	cerr << argv[0] << ": unknown exception" << endl;
	status = EXIT_FAILURE;
    }

    if (dbEnv)
    {
	try
	{
	    dbEnv->close();
	}
	catch(const LocalException& ex)
	{
	    cerr << argv[0] << ": local exception: " << ex << endl;
	    status = EXIT_FAILURE;
	}
	catch(const DBExceptionPtrE& ex)
	{
	    cerr << argv[0] << ": database exception: " << ex->message << endl;
	    status = EXIT_FAILURE;
	}
	catch(...)
	{
	    cerr << argv[0] << ": unknown exception" << endl;
	    status = EXIT_FAILURE;
	}
    }

    defaultInterrupt();
    cout << "shutting down..." << endl;

    return status;
}

int
main(int argc, char* argv[])
{
    PhoneBookServer app;
    return app.main(argc, argv, "config");
}
