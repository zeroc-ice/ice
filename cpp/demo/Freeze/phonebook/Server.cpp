// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Freeze/Application.h>
#include <ObjectFactory.h>

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
    string value;
    
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
    adapter->addServantLocator(evictor, "contact");
    
    //
    // Create the phonebook, and add it to the Object Adapter.
    //
    PhoneBookIPtr phoneBook = new PhoneBookI(adapter, dbPhoneBook, evictor);
    adapter->add(phoneBook, "phonebook");
    
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
    adapter->activate();
    shutdownOnInterrupt();
    communicator()->waitForShutdown();
    ignoreInterrupt();
    return EXIT_SUCCESS;
}
