// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <ContactFactory.h>
#include <NameIndex.h>
#include <PhoneBookI.h>
#include <Parser.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

class PhoneBookCollocated : public Ice::Application
{
public:
    
    PhoneBookCollocated(const string& envName) :
	_envName(envName)
    {
    }

    virtual int run(int argc, char* argv[]);

private:
    const string _envName;
};

int
main(int argc, char* argv[])
{
    PhoneBookCollocated app("db");
    return app.main(argc, argv, "config");
}

int
PhoneBookCollocated::run(int argc, char* argv[])
{
    PropertiesPtr properties = communicator()->getProperties();

    //
    // Create and install a factory for contacts.
    //
    ContactFactoryPtr contactFactory = new ContactFactory();
    communicator()->addObjectFactory(contactFactory, "::Demo::Contact");

    //
    // Create the name index.
    //
    NameIndexPtr index = new NameIndex("name");
    vector<Freeze::IndexPtr> indices;
    indices.push_back(index);

    //
    // Create an object adapter, use the evictor as servant locator.
    //
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("PhoneBook");

    //
    // Create an evictor for contacts.
    //
    Freeze::EvictorPtr evictor = Freeze::createEvictor(adapter, _envName, "contacts", 0, indices);
    adapter->addServantLocator(evictor, "contact");

    Int evictorSize = properties->getPropertyAsInt("PhoneBook.EvictorSize");
    if(evictorSize > 0)
    {
	evictor->setSize(evictorSize);
    }
    contactFactory->setEvictor(evictor);
    
    //
    // Create the phonebook, and add it to the Object Adapter.
    //
    PhoneBookIPtr phoneBook = new PhoneBookI(evictor, contactFactory, index);
    adapter->add(phoneBook, stringToIdentity("phonebook"));
    
    //
    // Everything ok, let's go.
    //
    int runParser(int, char*[], const CommunicatorPtr&);
    int status = runParser(argc, argv, communicator());
    adapter->deactivate();
    adapter->waitForDeactivate();

    return status;
}
