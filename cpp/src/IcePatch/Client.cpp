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
#include <Glacier/Glacier.h>
#include <IceUtil/Base64.h>
#include <IceSSL/Plugin.h>
#include <iomanip>
#ifdef _WIN32
#   include <direct.h>
#endif

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
    cerr << "Usage: " << appName() << " [options] [sub-directories...]\n";
    cerr <<     
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        ;
}

int
IcePatch::Client::run(int argc, char* argv[])
{
    Glacier::RouterPrx router;

    try
    {
	int idx = 1;
	while (idx < argc)
	{
            if (strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
            {
                usage();
                return EXIT_SUCCESS;
            }
            else if (strcmp(argv[idx], "-v") == 0 || strcmp(argv[idx], "--version") == 0)
            {
                cout << ICE_STRING_VERSION << endl;
                return EXIT_SUCCESS;
            }
            else if (argv[idx][0] == '-')
            {
                cerr << appName() << ": unknown option `" << argv[idx] << "'" << endl;
                usage();
                return EXIT_FAILURE;
            }
	    else
	    {
		++idx;
	    }
	}
	
	vector<string> subdirs;
	if (argc >= 1)
	{
	    for (int i = 1; i < argc; ++i)
	    {
		subdirs.push_back(argv[i]);
	    }
	}
	else
	{
	    subdirs.push_back(".");
	}
        
        PropertiesPtr properties = communicator()->getProperties();
        
        //
        // Do Glacier setup, if so requested.
        //
        const char* glacierStarterEndpointsProperty = "Glacier.Starter.Endpoints";
	string glacierStarterEndpoints = properties->getProperty(glacierStarterEndpointsProperty);
	if (!glacierStarterEndpoints.empty())
	{
	    ObjectPrx starterBase = communicator()->stringToProxy("Glacier/starter:" + glacierStarterEndpoints);
	    Glacier::StarterPrx starter = Glacier::StarterPrx::checkedCast(starterBase);
	    if (!starter)
	    {
		cerr << appName() << ": endpoints `" << glacierStarterEndpoints
		     << "' do not refer to a glacier router starter" << endl;
		return EXIT_FAILURE;
	    }

	    ByteSeq privateKey;
	    ByteSeq publicKey;
	    ByteSeq routerCert;

	    while (!router)
	    {
		string id;
		string pw;

		cout << "user id: " << flush;
		cin >> id;
		cout << "password: " << flush;
		cin >> pw;
		
		try
		{
		    router = starter->startRouter(id, pw, privateKey, publicKey, routerCert);
		}
		catch (const Glacier::CannotStartRouterException& ex)
		{
		    cerr << appName() << ": " << ex << ":\n" << ex.reason << endl;
		    return EXIT_FAILURE;
		}
		catch (const Glacier::InvalidPasswordException&)
		{
		    cout << "password is invalid, try again" << endl;
		}
	    }

	    string clientConfig = properties->getProperty("IceSSL.Client.Config");
	    if (!clientConfig.empty())
	    {
		string privateKeyBase64 = IceUtil::Base64::encode(privateKey);
		string publicKeyBase64  = IceUtil::Base64::encode(publicKey);
		string routerCertString = IceUtil::Base64::encode(routerCert);

                PluginManagerPtr pluginManager = communicator()->getPluginManager();
                PluginPtr plugin = pluginManager->getPlugin("IceSSL");
                IceSSL::PluginPtr sslPlugin = IceSSL::PluginPtr::dynamicCast(plugin);
                assert(sslPlugin);

		sslPlugin->setCertificateVerifier(IceSSL::Client, sslPlugin->getSingleCertVerifier(routerCert));
		sslPlugin->setRSAKeysBase64(IceSSL::Client, privateKeyBase64, publicKeyBase64);
		sslPlugin->addTrustedCertificateBase64(IceSSL::Client, routerCertString);
	    }

	    communicator()->setDefaultRouter(router);
	}
        
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
		return EXIT_FAILURE;
	    }
        }
        
        //
        // Create and install the node description factory.
        //
        ObjectFactoryPtr factory = new FileDescFactory;
        communicator()->addObjectFactory(factory, "::IcePatch::DirectoryDesc");
        communicator()->addObjectFactory(factory, "::IcePatch::RegularDesc");
        
	//
	// Patch all subdirectories.
	//
	for (vector<string>::const_iterator p = subdirs.begin(); p != subdirs.end(); ++p)
	{
	    Identity identity = pathToIdentity(*p);
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
    }
    catch (const FileAccessException& ex)
    {
	cout << endl; // There might still be a non-terminated line on cout.
	cerr << appName() << ": " << ex << ":\n" << ex.reason << endl;
	if (router)
	{
	    router->shutdown();
	}
	return EXIT_FAILURE;
    }
    catch (const BusyException&)
    {
	cout << endl; // There might still be a non-terminated line on cout.
	cerr << appName() << ": patching service busy, try again later" << endl;
	if (router)
	{
	    router->shutdown();
	}
	return EXIT_FAILURE;
    }
    
    if (router)
    {
	router->shutdown();
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

    virtual void startDownload(Int total, Int pos)
    {
	Ice::Int percent = pos * 100 / total;
	cout << " download " << setw(3) << percent << "%" << flush;
    }

    virtual void updateDownload(Int total, Int pos)
    {
	Ice::Int percent = pos * 100 / total;
	cout << "\b\b\b\b" << setw(3) << percent << "%" << flush;
    }

    virtual void finishedDownload(Int total)
    {
	updateDownload(total, total);
    }

    virtual void startUncompress(Int total, Int pos)
    {
	Ice::Int percent = pos * 100 / total;
	cout << " uncompress " << setw(3) << percent << "%" << flush;
    }
    
    virtual void updateUncompress(Int total, Int pos)
    {
	Ice::Int percent = pos * 100 / total;
	cout << "\b\b\b\b" << setw(3) << percent << "%" << flush;
    }

    virtual void finishedUncompress(Int total)
    {
	finishedDownload(total);
	cout << endl;
    }
};

void
IcePatch::Client::patch(const FileDescSeq& fileDescSeq, const string& indent)
{
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
	    cout << indent << "+-" << pathToName(path) << ":";

	    FileInfo info = getFileInfo(path, false);
	    switch (info.type)
	    {
		case FileTypeNotExist:
		{
		    createDirectory(path);
		    cout << " created" << endl;
		    break;
		}

		case FileTypeDirectory:
		{
		    cout << " ok" << endl;
		    break;
		}
		
		case FileTypeRegular:
		{
		    removeRecursive(path);
		    createDirectory(path);
		    cout << " created" << endl;
		    break;
		}

		case FileTypeUnknown:
		{
		    removeRecursive(path);
		    createDirectory(path);
		    cout << " created" << endl;
		    break;
		}
	    }

	    cout << newIndent << "|" << endl;
	    patch(directoryDesc->directory->getContents(), newIndent);
	}
	else
	{
	    assert(regularDesc);
	    cout << indent << "+-" << pathToName(path) << ":";

	    MyProgressCB progressCB;

	    FileInfo info = getFileInfo(path, false);
	    switch (info.type)
	    {
		case FileTypeNotExist:
		{
		    getRegular(regularDesc->regular, progressCB);
		    break;
		}

		case FileTypeDirectory:
		{
		    removeRecursive(path);
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
			removeRecursive(path);
			getRegular(regularDesc->regular, progressCB);
		    }
		    else
		    {
			cout << " ok" << endl;
		    }

		    break;
		}

		case FileTypeUnknown:
		{
		    removeRecursive(path);
		    getRegular(regularDesc->regular, progressCB);
		    break;
		}
	    }

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
    PropertiesPtr defaultProperties;
    try
    {
	defaultProperties = getDefaultProperties(argc, argv);
        StringSeq args = argsToStringSeq(argc, argv);
        args = defaultProperties->parseCommandLineOptions("IcePatch", args);
        args = defaultProperties->parseCommandLineOptions("Glacier", args);
        stringSeqToArgs(args, argc, argv);
    }
    catch(const Exception& ex)
    {
	cerr << argv[0] << ": " << ex << endl;
	return EXIT_FAILURE;
    }

    Client app;
    return app.main(argc, argv);
}
