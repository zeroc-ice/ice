// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Evictor.h>
#include <ServantFactory.h>

using namespace Ice;
using namespace Freeze;
using namespace std;

int
run(int argc, char* argv[], const CommunicatorPtr& communicator, const DBEnvPtr& dbenv)
{
    ObjectAdapterPtr adapter = communicator->createObjectAdapter("PhoneBookAdapter");
    DBPtr db = dbenv->open("phonebook");
    EvictorPtr evictor = new Evictor(db, 3); // TODO: Evictor size must be configurable
    adapter->setServantLocator(evictor);

    ServantFactoryPtr phoneBookFactory = new PhoneBookFactory(adapter, evictor);
    communicator->installServantFactory(phoneBookFactory, "::PhoneBook");

    PhoneBookIPtr phoneBook;
    ObjectPtr servant = db->get("phonebook");
    if (!servant)
    {
	phoneBook = new PhoneBookI(adapter, evictor);
    }
    else
    {
	phoneBook = PhoneBookIPtr::dynamicCast(servant);
    }
    assert(phoneBook);
    adapter->add(phoneBook, "phonebook");

    ServantFactoryPtr contactFactory = new ContactFactory(phoneBook, evictor);
    communicator->installServantFactory(contactFactory, "::Contact");

    adapter->activate();
    communicator->waitForShutdown();

    db->put("phonebook", phoneBook);

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    CommunicatorPtr communicator;
    DBEnvPtr dbenv;

    try
    {
	PropertiesPtr properties = createPropertiesFromFile(argc, argv, "config");
	communicator = Ice::initializeWithProperties(properties);
	dbenv = Freeze::initializeWithProperties(communicator, properties);
	status = run(argc, argv, communicator, dbenv);
    }
    catch(const LocalException& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }
    catch(const DBException& ex)
    {
	cerr << "Berkeley DB error: " << ex.message << endl;
	status = EXIT_FAILURE;
    }

    if (dbenv)
    {
	try
	{
	    dbenv->close();
	}
	catch(const LocalException& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
	catch(const DBException& ex)
	{
	    cerr << "Berkeley DB error: " << ex.message << endl;
	    status = EXIT_FAILURE;
	}
    }

    if (communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const LocalException& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
