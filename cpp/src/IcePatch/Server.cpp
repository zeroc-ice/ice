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

#include <IceUtil/IceUtil.h>
#include <Ice/Application.h>
#include <IcePatch/FileLocator.h>
#include <IcePatch/IcePatchI.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

namespace IcePatch
{

class Server : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

class Updater : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    Updater(const ObjectAdapterPtr&, const IceUtil::Time&);

    virtual void run();
    void destroy();

protected:

    const ObjectAdapterPtr _adapter;
    const LoggerPtr _logger;
    const IceUtil::Time _updatePeriod;
    bool _destroy;

    void cleanup(const FileDescSeq&);
};

typedef IceUtil::Handle<Updater> UpdaterPtr;

};

void
IcePatch::Server::usage()
{
    cerr << "Usage: " << appName() << " [options]\n";
    cerr <<     
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        ;
}

int
IcePatch::Server::run(int argc, char* argv[])
{
    for(int i = 1; i < argc; ++i)
    {
	if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
	{
	    usage();
	    return EXIT_SUCCESS;
	}
	else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
	{
	    cout << ICE_STRING_VERSION << endl;
	    return EXIT_SUCCESS;
	}
	else
	{
	    cerr << appName() << ": unknown option `" << argv[i] << "'" << endl;
	    usage();
	    return EXIT_FAILURE;
	}
    }
    
    PropertiesPtr properties = communicator()->getProperties();
    
    //
    // Get the IcePatch endpoints.
    //
    const char* endpointsProperty = "IcePatch.Endpoints";
    if(properties->getProperty(endpointsProperty).empty())
    {
	cerr << appName() << ": property `" << endpointsProperty << "' is not set" << endl;
	return EXIT_FAILURE;
    }
    
    //
    // Create and initialize the object adapter and the file locator.
    //
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("IcePatch");
    ServantLocatorPtr fileLocator = new FileLocator(adapter);
    adapter->addServantLocator(fileLocator, "IcePatch");

    //
    // Start the updater if an update period is set.
    //
    UpdaterPtr updater;
    IceUtil::Time updatePeriod = IceUtil::Time::seconds(
	properties->getPropertyAsIntWithDefault("IcePatch.UpdatePeriod", 60));
    if(updatePeriod != IceUtil::Time())
    {
	if(updatePeriod < IceUtil::Time::seconds(1))
	{
	    updatePeriod = IceUtil::Time::seconds(1);
	}
	updater = new Updater(adapter, updatePeriod);
	updater->start();
    }

    //
    // Everything ok, let's go.
    //
    shutdownOnInterrupt();
    adapter->activate();
    communicator()->waitForShutdown();
    ignoreInterrupt();

    //
    // Destroy and join with the updater, if there is one.
    //
    if(updater)
    {
	updater->destroy();
	updater->getThreadControl().join();
    }

    return EXIT_SUCCESS;
}

IcePatch::Updater::Updater(const ObjectAdapterPtr& adapter, const IceUtil::Time& updatePeriod) :
    _adapter(adapter),
    _logger(_adapter->getCommunicator()->getLogger()),
    _updatePeriod(updatePeriod),
    _destroy(false)
{
}

void
IcePatch::Updater::run()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    while(!_destroy)
    {
	try
	{
	    Identity ident;
	    ident.category = "IcePatch";
	    ident.name = ".";
	    ObjectPrx topObj = _adapter->createProxy(ident);
	    FilePrx top = FilePrx::checkedCast(topObj);
	    assert(top);
	    DirectoryDescPtr topDesc = DirectoryDescPtr::dynamicCast(top->describe());
	    assert(topDesc);
	    cleanup(topDesc->dir->getContents());
	}
	catch(const FileAccessException& ex)
	{
	    Error out(_logger);
	    out << "exception during update:\n" << ex << ":\n" << ex.reason;
	}
	catch(const BusyException&)
	{
	    //
	    // Just loop if we're busy.
	    //
	}
	catch(const Exception& ex)
	{
	    //
	    // Log other exceptions only if we are not destroyed.
	    //
	    if(!_destroy)
	    {
		Error out(_logger);
		out << "exception during update:\n" << ex;
	    }
	}

	if(_destroy)
	{
	    break;
	}

	timedWait(_updatePeriod);
    }
}

void
IcePatch::Updater::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _destroy = true;
    notify();
}

void
IcePatch::Updater::cleanup(const FileDescSeq& fileDescSeq)
{
    for(FileDescSeq::const_iterator p = fileDescSeq.begin(); p != fileDescSeq.end(); ++p)
    {
	if(_destroy)
	{
	    return;
	}

	DirectoryDescPtr directoryDesc = DirectoryDescPtr::dynamicCast(*p);
	if(directoryDesc)
	{
	    //
	    // Force MD5 files to be created and orphaned files to be
	    // removed. Then recurse into subdirectories.
	    //
	    cleanup(directoryDesc->dir->getContents());

	    //
	    // Call describe(), because BZ2 and MD5 files in the
	    // directory might have changed, resulting in a different
	    // summary MD5 for this directory.
	    //
	    directoryDesc->dir->describe();
	}
	else
	{
	    RegularDescPtr regularDesc = RegularDescPtr::dynamicCast(*p);
	    assert(regularDesc);

	    //
	    // Force BZ2 files to be created for all regular files.
	    //
	    regularDesc->reg->getBZ2Size();
	}
    }
}

int
main(int argc, char* argv[])
{
    Server app;
    return app.main(argc, argv);
}
