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

#include <Freeze/Application.h>
#include <ContactFactory.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

class PhoneBookServer : public Freeze::Application
{
public:
    
    PhoneBookServer(const string& dbEnvName) :
	Freeze::Application(dbEnvName)
    {
    }

    virtual int runFreeze(int argc, char* argv[], const DBEnvironmentPtr&);
};

int
main(int argc, char* argv[])
{
    PhoneBookServer app("db");
    return app.main(argc, argv, "config");
}

int
PhoneBookServer::runFreeze(int argc, char* argv[], const DBEnvironmentPtr& dbEnv)
{
    PropertiesPtr properties = communicator()->getProperties();
    
    DBPtr dbPhoneBook = dbEnv->openDB("phonebook", true);
    DBPtr dbContacts = dbEnv->openDB("contacts", true);
    
    //
    // Create an Evictor for contacts.
    //
    PersistenceStrategyPtr strategy;
    if(properties->getPropertyAsInt("PhoneBook.IdleStrategy") > 0)
    {
        strategy = dbContacts->createIdleStrategy();
    }
    else
    {
        strategy = dbContacts->createEvictionStrategy();
    }
    EvictorPtr evictor = dbContacts->createEvictor(strategy);
    Int evictorSize = properties->getPropertyAsInt("PhoneBook.EvictorSize");
    if(evictorSize > 0)
    {
	evictor->setSize(evictorSize);
    }
    
    //
    // Create an Object Adapter, use the Evictor as Servant Locator.
    //
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("PhoneBook");
    adapter->addServantLocator(evictor, "contact");
    
    //
    // Create the phonebook, and add it to the Object Adapter.
    //
    PhoneBookIPtr phoneBook = new PhoneBookI(adapter, dbPhoneBook, evictor);
    adapter->add(phoneBook, stringToIdentity("phonebook"));
    
    //
    // Create and install a factory and initializer for contacts.
    //
    ObjectFactoryPtr contactFactory = new ContactFactory(phoneBook, evictor);
    ServantInitializerPtr contactInitializer = ServantInitializerPtr::dynamicCast(contactFactory);
    communicator()->addObjectFactory(contactFactory, "::Contact");
    evictor->installServantInitializer(contactInitializer);
    
    //
    // Everything ok, let's go.
    //
    shutdownOnInterrupt();
    adapter->activate();
    communicator()->waitForShutdown();
    ignoreInterrupt();

    return EXIT_SUCCESS;
}
