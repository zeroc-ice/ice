// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <ServantFactory.h>
#include <ServantInitializer.h>

static Ice::CommunicatorPtr communicator;

#ifdef WIN32

static BOOL WINAPI
interruptHandler(DWORD)
{
    assert(communicator);
    communicator->shutdown();
    return TRUE;
}

static void
shutdownOnInterrupt()
{
    SetConsoleCtrlHandler(NULL, FALSE);
    SetConsoleCtrlHandler(interruptHandler, TRUE);
}

static void
ignoreInterrupt()
{
    SetConsoleCtrlHandler(NULL, TRUE);
    SetConsoleCtrlHandler(interruptHandler, FALSE);
}

#else

#   include <signal.h>

static void
interruptHandler(int)
{
    assert(communicator);
    communicator->shutdown();
}

static void
shutdownOnInterrupt()
{
    struct sigaction action;
    action.sa_handler = interruptHandler;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGHUP);
    sigaddset(&action.sa_mask, SIGINT);
    sigaddset(&action.sa_mask, SIGTERM);
    action.sa_flags = 0;
    sigaction(SIGHUP, &action, 0);
    sigaction(SIGINT, &action, 0);
    sigaction(SIGTERM, &action, 0);
}

static void
ignoreInterrupt()
{
    struct sigaction action;
    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGHUP, &action, 0);
    sigaction(SIGINT, &action, 0);
    sigaction(SIGTERM, &action, 0);
}

#endif

using namespace Ice;
using namespace Freeze;
using namespace std;

int
run(int argc, char* argv[], const DBEnvPtr& dbenv)
{
    cout << "starting up..." << endl;
    ignoreInterrupt();

    DBPtr db = dbenv->open("phonebook");

    EvictorPtr evictor = db->createEvictor();
    evictor->setSize(3);
    
    ObjectAdapterPtr adapter = communicator->createObjectAdapter("PhoneBookAdapter");
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

    ServantInitializerPtr contactInitializer = new ContactInitializer(phoneBook);
    evictor->installServantInitializer(contactInitializer);

    adapter->activate();

    shutdownOnInterrupt();
    communicator->waitForShutdown();
    cout << "shutting down..." << endl;
    ignoreInterrupt();

    db->put("phonebook", phoneBook);

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    DBEnvPtr dbenv;

    try
    {
	PropertiesPtr properties = createPropertiesFromFile(argc, argv, "config");
	communicator = Ice::initializeWithProperties(properties);
	dbenv = Freeze::initialize(communicator, "db");
	status = run(argc, argv, dbenv);
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
	    communicator = 0;
	}
	catch(const LocalException& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
