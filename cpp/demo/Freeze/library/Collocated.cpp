// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Freeze/Freeze.h>
#include <BookFactory.h>
#include <Parser.h>

using namespace std;

class LibraryCollocated : public Ice::Application
{
public:
    
    LibraryCollocated(const string&);

    virtual int run(int argc, char* argv[]);

private:

    const string _envName;
};

int
main(int argc, char* argv[])
{
    LibraryCollocated app("db");
    return app.main(argc, argv, "config.collocated");
}

LibraryCollocated::LibraryCollocated(const string& envName) :
    //
    // Since this is an interactive demo we don't want any signal
    // handling.
    //
    Ice::Application(Ice::NoSignalHandling),
    _envName(envName)
{
}

int
LibraryCollocated::run(int argc, char* argv[])
{
    Ice::PropertiesPtr properties = communicator()->getProperties();
    
    //
    // Create an object adapter
    //
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Library");

    //
    // Create an evictor for books.
    //
    Freeze::EvictorPtr evictor = Freeze::createBackgroundSaveEvictor(adapter, _envName, "books");
    Ice::Int evictorSize = properties->getPropertyAsInt("EvictorSize");
    if(evictorSize > 0)
    {
        evictor->setSize(evictorSize);
    }
    
    //
    // Use the evictor as servant Locator.
    //
    adapter->addServantLocator(evictor, "book");
    
    //
    // Create the library, and add it to the Object Adapter.
    //
    LibraryIPtr library = new LibraryI(communicator(), _envName, "authors", evictor);
    adapter->add(library, communicator()->stringToIdentity("library"));
    
    //
    // Create and install a factory for books.
    //
    Ice::ObjectFactoryPtr bookFactory = new BookFactory(library);
    communicator()->addObjectFactory(bookFactory, Demo::Book::ice_staticId());

    //
    // Everything ok, let's go.
    //
    int runParser(int, char*[], const Ice::CommunicatorPtr&);
    int status = runParser(argc, argv, communicator());
    adapter->destroy();

    return status;
}
