// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Freeze/Application.h>
#include <BookFactory.h>
#include <Parser.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

class LibraryCollocated : public Freeze::Application
{
public:
    
    LibraryCollocated(const string& dbEnvName) :
	Freeze::Application(dbEnvName)
    {
    }

    virtual int runFreeze(int argc, char* argv[], const DBEnvironmentPtr&);
};

int
main(int argc, char* argv[])
{
    LibraryCollocated app("db");
    return app.main(argc, argv, "config");
}

int
LibraryCollocated::runFreeze(int argc, char* argv[], const DBEnvironmentPtr& dbEnv)
{
    PropertiesPtr properties = communicator()->getProperties();
    string value;
    
    DBPtr dbBooks = dbEnv->openDB("books", true);
    DBPtr dbAuthors = dbEnv->openDB("authors", true);
    
    //
    // Create an Evictor for books.
    //
    EvictorPtr evictor;
    if(properties->getPropertyAsInt("Library.SaveAfterMutatingOperation") > 0)
    {
	evictor = dbBooks->createEvictor(SaveAfterMutatingOperation);
    }
    else
    {
	evictor = dbBooks->createEvictor(SaveUponEviction);
    }
    Int evictorSize = properties->getPropertyAsInt("Library.EvictorSize");
    if(evictorSize > 0)
    {
	evictor->setSize(evictorSize);
    }
    
    //
    // Create an Object Adapter, use the Evictor as Servant Locator.
    //
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("LibraryAdapter");
    adapter->addServantLocator(evictor, "book");
    
    //
    // Create the library, and add it to the Object Adapter.
    //
    LibraryIPtr library = new LibraryI(adapter, dbAuthors, evictor);
    adapter->add(library, stringToIdentity("library"));
    
    //
    // Create and install a factory and initializer for books.
    //
    ObjectFactoryPtr bookFactory = new BookFactory(library);
    communicator()->addObjectFactory(bookFactory, "::Book");

    //
    // Everything ok, let's go.
    //
    int runParser(int, char*[], const CommunicatorPtr&);
    int status = runParser(argc, argv, communicator());
    adapter->deactivate();

    return status;
}
