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

#include <Ice/Application.h>
#include <BookFactory.h>
#include <Freeze/Freeze.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

class LibraryServer : public Ice::Application
{
public:
    
    LibraryServer(const string& envName) :
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
    LibraryServer app("db");
    return app.main(argc, argv, "config");
}

int
LibraryServer::run(int argc, char* argv[])
{
    PropertiesPtr properties = communicator()->getProperties();

    //
    // Create an Evictor for books.
    //
    Freeze::EvictorPtr evictor = Freeze::createEvictor(communicator(), _envName, "books");

    Int evictorSize = properties->getPropertyAsInt("Library.EvictorSize");
    if(evictorSize > 0)
    {
	evictor->setSize(evictorSize);
    }
    
    //
    // Create an Object Adapter, use the Evictor as Servant Locator.
    //
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Library");
    adapter->addServantLocator(evictor, "book");
    
    //
    // Create the library, and add it to the Object Adapter.
    //
    LibraryIPtr library = new LibraryI(communicator(), _envName, "authors", evictor);
    adapter->add(library, stringToIdentity("library"));
    
    //
    // Create and install a factory and initializer for books.
    //
    ObjectFactoryPtr bookFactory = new BookFactory(library);
    communicator()->addObjectFactory(bookFactory, "::Book");
    
    //
    // Everything ok, let's go.
    //
    shutdownOnInterrupt();
    adapter->activate();
    communicator()->waitForShutdown();
    ignoreInterrupt();

    return EXIT_SUCCESS;
}
