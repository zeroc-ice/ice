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

    static void removeOrphanedRecursive(const string&);
    static void updateRecursive(const string&);
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
            changeDirectory(directory);
        }

	//
	// Remove orphaned MD5 and BZ2 files.
        // Create MD5 and BZ2 files.
	//
	removeOrphanedRecursive(".");
	updateRecursive(".");

        //
        // Create and initialize the object adapter and the file locator.
        //
        ObjectAdapterPtr adapter = communicator()->createObjectAdapterFromProperty("IcePatch", endpointsProperty);
        ServantLocatorPtr fileLocator = new FileLocator(adapter);
        adapter->addServantLocator(fileLocator, "IcePatch");
        adapter->activate();
         
        //
        // We're done, let's wait for shutdown.
        //
        communicator()->waitForShutdown();
    }
    catch (const FileAccessException& ex)
    {
	cerr << appName() << ": " << ex << ":\n" << ex.reason << endl;
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void
IcePatch::Server::removeOrphanedRecursive(const string& path)
{
    assert(getFileInfo(path, true).type == FileTypeDirectory);
    
    StringSeq paths = readDirectory(path);
    StringSeq::const_iterator p;
    for (p = paths.begin(); p != paths.end(); ++p)
    {
	if (ignoreSuffix(*p))
 	{
	    pair<StringSeq::const_iterator, StringSeq::const_iterator> r =
		equal_range(paths.begin(), paths.end(), removeSuffix(*p));
	    if (r.first == r.second)
	    {
		cout << "removing orphaned file `" << *p << "'... " << flush;
		removeRecursive(*p);
		cout << "ok" << endl;
	    }
	}
	else
	{
	    if (getFileInfo(*p, true).type == FileTypeDirectory)
	    {
		removeOrphanedRecursive(*p);
	    }
	}
    }

    if (readDirectory(path).empty())
    {
	cout << "removing empty directory `" << *p << "'... " << flush;
	removeRecursive(path);
	cout << "ok" << endl;
    }
}

void
IcePatch::Server::updateRecursive(const string& path)
{
    if (ignoreSuffix(path))
    {
	return;
    }

    FileInfo info = getFileInfo(path, true);

    if (info.type == FileTypeDirectory)
    {
	StringSeq paths = readDirectory(path);
	StringSeq::const_iterator p;
	for (p = paths.begin(); p != paths.end(); ++p)
	{
	    updateRecursive(*p);
	}
    }
    else if (info.type == FileTypeRegular)
    {
	FileInfo infoMD5 = getFileInfo(path + ".md5", false);
	if (infoMD5.type != FileTypeRegular || infoMD5.time < info.time)
	{
	    cout << "creating .md5 file for `" << path << "'... " << flush;
	    createMD5(path);
	    cout << "ok" << endl;
	}

	FileInfo infoBZ2 = getFileInfo(path + ".bz2", false);
	if (infoBZ2.type != FileTypeRegular || infoBZ2.time < info.time)
	{
	    cout << "creating .bz2 file for `" << path << "'... " << flush;
	    createBZ2(path);
	    cout << "ok" << endl;
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
