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
#include <BookFactory.h>

using namespace std;

class LibraryServer : public Freeze::Application
{
public:
    
    LibraryServer(const string& dbEnvName) :
	Freeze::Application(dbEnvName)
    {
    }

    virtual int runFreeze(int argc, char* argv[], const Freeze::DBEnvironmentPtr&);
};

int
main(int argc, char* argv[])
{
    LibraryServer app("db");
    return app.main(argc, argv, "config");
}

int
LibraryServer::runFreeze(int argc, char* argv[], const Freeze::DBEnvironmentPtr& dbEnv)
{
    Ice::PropertiesPtr properties = communicator()->getProperties();
    string value;
    
    Freeze::DBPtr dbBooks = dbEnv->openDB("books", true);
    Freeze::DBPtr dbAuthors = dbEnv->openDB("authors", true);
    
    //
    // Create an Evictor for books.
    //
    Freeze::EvictorPersistenceMode mode;
    value = properties->getProperty("Library.SaveAfterMutatingOperation");
    if(!value.empty() && atoi(value.c_str()) > 0)
    {
	mode = Freeze::SaveAfterMutatingOperation;
    }
    else
    {
	mode = Freeze::SaveUponEviction;
    }

    Freeze::EvictorPtr evictor = dbBooks->createEvictor(mode);
    value = properties->getProperty("Library.EvictorSize");
    if(!value.empty())
    {
	evictor->setSize(atoi(value.c_str()));
    }
    
    //
    // Create an Object Adapter, use the Evictor as Servant Locator.
    //
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("LibraryAdapter");
    adapter->addServantLocator(evictor, "book");
    
    //
    // Create the library, and add it to the Object Adapter.
    //
    LibraryIPtr library = new LibraryI(adapter, dbAuthors, evictor);
    adapter->add(library, Ice::stringToIdentity("library"));
    
    //
    // Create and install a factory and initializer for books.
    //
    Ice::ObjectFactoryPtr bookFactory = new BookFactory(library, evictor);
    communicator()->addObjectFactory(bookFactory, "::Book");
    
    //
    // Everything ok, let's go.
    //
    adapter->activate();
    shutdownOnInterrupt();
    communicator()->waitForShutdown();
    ignoreInterrupt();

    return EXIT_SUCCESS;
}
