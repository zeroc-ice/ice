// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Application.h>
#include <IcePatch/FileLocator.h>
#include <IcePatch/Util.h>
#ifdef _WIN32
#   include <direct.h>
#endif

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

    static void cleanup(const FileDescSeq&);
};

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
    try
    {
        for (int i = 1; i < argc; ++i)
        {
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
            {
                usage();
                return EXIT_SUCCESS;
            }
            else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
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
        string endpoints = properties->getProperty(endpointsProperty);
        if (endpoints.empty())
        {
            cerr << appName() << ": property `" << endpointsProperty << "' is not set" << endl;
            return EXIT_FAILURE;
        }
        
        //
        // Get the working directory and change to this directory.
        //
        const char* directoryProperty = "IcePatch.Directory";
        string directory = properties->getProperty(directoryProperty);
        if (!directory.empty())
        {
#ifdef _WIN32
	    if (_chdir(directory.c_str()) == -1)
#else
	    if (chdir(directory.c_str()) == -1)
#endif
	    {
		cerr << appName() << ": cannot change to directory `" << directory << "': " << strerror(errno) << endl;
	    }
        }

        //
        // Create and initialize the object adapter and the file locator.
        //
        ObjectAdapterPtr adapter = communicator()->createObjectAdapterFromProperty("IcePatch", endpointsProperty);
        ServantLocatorPtr fileLocator = new FileLocator(adapter);
        adapter->addServantLocator(fileLocator, "IcePatch");
         
	//
	// Do cleanup.
	//
	Identity identity = pathToIdentity(".");
	ObjectPrx topObj = communicator()->stringToProxy(identityToString(identity) + ':' + endpoints);
	FilePrx top = FilePrx::checkedCast(topObj);
	assert(top);
	DirectoryDescPtr topDesc = DirectoryDescPtr::dynamicCast(top->describe());
	assert(topDesc);
	cleanup(topDesc->directory->getContents());

	//
	// Everything ok, let's go.
	//
	shutdownOnInterrupt();
	adapter->activate();
	communicator()->waitForShutdown();
	ignoreInterrupt();
    }
    catch (const FileAccessException& ex)
    {
	cerr << appName() << ": " << ex << ":\n" << ex.reason << endl;
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void
IcePatch::Server::cleanup(const FileDescSeq& fileDescSeq)
{
    for (FileDescSeq::const_iterator p = fileDescSeq.begin(); p != fileDescSeq.end(); ++p)
    {
	DirectoryDescPtr directoryDesc = DirectoryDescPtr::dynamicCast(*p);
	if (directoryDesc)
	{
	    //
	    // Force .md5 files to be created and orphaned files to be
	    // removed.
	    //
	    cleanup(directoryDesc->directory->getContents());
	}
	else
	{
	    RegularDescPtr regularDesc = RegularDescPtr::dynamicCast(*p);
	    assert(regularDesc);

	    //
	    // Force .bz2 files to be created.
	    //
	    regularDesc->regular->getBZ2Size();
	}
    }
}

int
main(int argc, char* argv[])
{
    addArgumentPrefix("IcePatch");
    Server app;
    return app.main(argc, argv);
}
