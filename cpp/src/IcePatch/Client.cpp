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
#include <IcePatch/FileDescFactory.h>
#include <IcePatch/Util.h>
#include <iomanip>

using namespace std;
using namespace Ice;
using namespace IcePatch;

namespace IcePatch
{

class Client : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);

    static string pathToName(const string&);
    static void patch(const FileDescSeq&, const string&);
};

};

void
IcePatch::Client::usage()
{
    cerr << "Usage: " << appName() << " [options]\n";
    cerr <<     
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        ;
}

int
IcePatch::Client::run(int argc, char* argv[])
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
        // Create and install the node description factory.
        //
        ObjectFactoryPtr factory = new FileDescFactory;
        communicator()->addObjectFactory(factory, "::IcePatch::DirectoryDesc");
        communicator()->addObjectFactory(factory, "::IcePatch::RegularDesc");
        
	//
	// Patch all files.
	//
	Identity identity = pathToIdentity(".");
	ObjectPrx topObj = communicator()->stringToProxy(identityToString(identity) + ':' + endpoints);
	FilePrx top = FilePrx::checkedCast(topObj);
	assert(top);
	DirectoryDescPtr topDesc = DirectoryDescPtr::dynamicCast(top->describe());
	assert(topDesc);
	string path = identityToPath(topDesc->directory->ice_getIdentity());
	cout << pathToName(path) << endl;
	cout << "|" << endl;
	patch(topDesc->directory->getContents(), "");
    }
    catch (const FileAccessException& ex)
    {
	cerr << appName() << ": " << ex << ":\n" << ex.reason << endl;
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

string
IcePatch::Client::pathToName(const string& path)
{
    string::size_type pos = path.rfind('/');
    if (pos == string::npos)
    {
	return path;
    }
    else
    {
	return path.substr(pos + 1);
    }
}

class MyProgressCB : public ProgressCB
{
public:

    virtual void startDownload(Int)
    {
	cout << "download   0% " << flush;
    }

    virtual void updateDownload(Int total, Int pos)
    {
	Ice::Int percent = pos * 100 / total;
	cout << "\b\b\b\b\b" << setw(3) << percent << "% " << flush;
    }

    virtual void finishedDownload(Int total)
    {
	cout << "\b\b\b\b\b" << "100% " << flush;
    }

    virtual void startUncompress(Int)
    {
	cout << "uncompress   0% " << flush;
    }

    virtual void updateUncompress(Int total, Int pos)
    {
	updateDownload(total, pos);
    }

    virtual void finishedUncompress(Int total)
    {
	finishedDownload(total);
    }
};

void
IcePatch::Client::patch(const FileDescSeq& fileDescSeq, const string& indent)
{
    if (fileDescSeq.empty())
    {
	return;
    }

    for (unsigned int i = 0; i < fileDescSeq.size(); ++i)
    {
	string path;
	DirectoryDescPtr directoryDesc = DirectoryDescPtr::dynamicCast(fileDescSeq[i]);
	RegularDescPtr regularDesc;
	if (directoryDesc)
	{
	    path = identityToPath(directoryDesc->directory->ice_getIdentity());
	}
	else
	{
	    regularDesc = RegularDescPtr::dynamicCast(fileDescSeq[i]);
	    assert(regularDesc);
	    path = identityToPath(regularDesc->regular->ice_getIdentity());
	}

	bool last = (i == fileDescSeq.size() - 1);
	
	if (directoryDesc)
	{
	    string newIndent;
	    if (last)
	    {
		newIndent = indent + "  ";
	    }
	    else
	    {
		newIndent = indent + "| ";
	    }
	    cout << indent << "+-" << pathToName(path) << ": " << flush;

	    FileInfo info = getFileInfo(path, false);
	    switch (info.type)
	    {
		case FileTypeNotExist:
		{
		    cout << "creating directory... " << flush;
		    createDirectory(path);
		    break;
		}

		case FileTypeDirectory:
		{
		    break;
		}

		case FileTypeRegular:
		{
		    cout << "removing file... " << flush;
		    removeRecursive(path);
		    cout << "creating directory... " << flush;
		    createDirectory(path);
		    break;
		}

		case FileTypeUnknown:
		{
		    cout << "removing unknown file... " << flush;
		    removeRecursive(path);
		    cout << "creating directory... " << flush;
		    createDirectory(path);
		    break;
		}
	    }

	    cout << "ok" << endl;

	    cout << newIndent << "|" << endl;
	    patch(directoryDesc->directory->getContents(), newIndent);
	}
	else
	{
	    assert(regularDesc);
	    cout << indent << "+-" << pathToName(path) << ": " << flush;

	    MyProgressCB progressCB;

	    FileInfo info = getFileInfo(path, false);
	    switch (info.type)
	    {
		case FileTypeNotExist:
		{
		    cout << "getting file... " << flush;
		    getRegular(regularDesc->regular, progressCB);
		    break;
		}

		case FileTypeDirectory:
		{
		    cout << "removing directory... " << flush;
		    removeRecursive(path);
		    cout << "getting file... " << flush;
		    getRegular(regularDesc->regular, progressCB);
		    break;
		}

		case FileTypeRegular:
		{
		    ByteSeq md5;
		    
		    string pathMD5 = path + ".md5";
		    FileInfo infoMD5 = getFileInfo(pathMD5, false);
		    if (infoMD5.type == FileTypeRegular && infoMD5.time >= info.time)
		    {
			md5 = getMD5(path);
		    }

		    if (md5 != regularDesc->md5)
		    {
			cout << "removing file... " << flush;
			removeRecursive(path);
			cout << "getting file... " << flush;
			getRegular(regularDesc->regular, progressCB);
		    }

		    break;
		}

		case FileTypeUnknown:
		{
		    cout << "removing unknown file... " << flush;
		    removeRecursive(path);
		    cout << "getting file... " << flush;
		    getRegular(regularDesc->regular, progressCB);
		    break;
		}
	    }

	    cout << "ok" << endl;

	    if (last)
	    {
		cout << indent << endl;
	    }
	}
    }
}

int
main(int argc, char* argv[])
{
    addArgumentPrefix("IcePatch");
    Client app;
    return app.main(argc, argv);
}
