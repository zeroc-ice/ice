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
#include <IcePatch/FileDescFactory.h>
#include <IcePatch/Util.h>
#include <IcePatch/ClientUtil.h>
#include <Glacier/Glacier.h>
#include <IceUtil/Base64.h>
#include <IceSSL/Plugin.h>
#include <iomanip>
#ifdef _WIN32
#   include <direct.h>
#endif
#include <set>

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

private:

    void patch(const DirectoryDescPtr&, const string&) const;

    bool _remove;
    bool _thorough;
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
	while(idx < argc)
	{
            if(strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
            {
                usage();
                return EXIT_SUCCESS;
            }
            else if(strcmp(argv[idx], "-v") == 0 || strcmp(argv[idx], "--version") == 0)
            {
                cout << ICE_STRING_VERSION << endl;
                return EXIT_SUCCESS;
            }
            else if(argv[idx][0] == '-')
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
	if(argc > 1)
	{
	    for(int i = 1; i < argc; ++i)
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
	if(!glacierStarterEndpoints.empty())
	{
	    ObjectPrx starterBase = communicator()->stringToProxy("Glacier/starter:" + glacierStarterEndpoints);
	    Glacier::StarterPrx starter = Glacier::StarterPrx::checkedCast(starterBase);
	    if(!starter)
	    {
		cerr << appName() << ": endpoints `" << glacierStarterEndpoints
		     << "' do not refer to a glacier router starter" << endl;
		return EXIT_FAILURE;
	    }

	    ByteSeq privateKey;
	    ByteSeq publicKey;
	    ByteSeq routerCert;

	    while(!router)
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
		catch(const Glacier::CannotStartRouterException& ex)
		{
		    cerr << appName() << ": " << ex << ":\n" << ex.reason << endl;
		    return EXIT_FAILURE;
		}
		catch(const Glacier::InvalidPasswordException&)
		{
		    cout << "password is invalid, try again" << endl;
		}
	    }

	    string clientConfig = properties->getProperty("IceSSL.Client.Config");
	    if(!clientConfig.empty())
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
        if(endpoints.empty())
        {
            cerr << appName() << ": property `" << endpointsProperty << "' is not set" << endl;
            return EXIT_FAILURE;
        }
        
        //
        // Get the working directory and change to this directory.
        //
        const char* directoryProperty = "IcePatch.Directory";
        string directory = properties->getProperty(directoryProperty);
        if(!directory.empty())
        {
#ifdef _WIN32
	    if(_chdir(directory.c_str()) == -1)
#else
	    if(chdir(directory.c_str()) == -1)
#endif
	    {
		cerr << appName() << ": cannot change to directory `" << directory << "': " << strerror(errno) << endl;
		return EXIT_FAILURE;
	    }
        }
        
	//
	// Check whether we want to remove orphaned files.
	//
	_remove = properties->getPropertyAsInt("IcePatch.RemoveOrphaned") > 0;
	if(_remove)
	{
#ifdef _WIN32
	    char cwd[_MAX_PATH];
	    _getcwd(cwd, _MAX_PATH);
#else
	    char cwd[PATH_MAX];
	    getcwd(cwd, PATH_MAX);
#endif
	    cout << "WARNING: All orphaned files in `" << cwd << "' will be removed." << endl;
	    string answer;
	    do
	    {
		cout << "Do you want to proceed? (yes/no)" << endl;
		cin >> answer;
		transform(answer.begin(), answer.end(), answer.begin(), ::tolower);
		if(answer == "no")
		{
		    return EXIT_SUCCESS;
		}
	    }
	    while(answer != "yes");
	}	

	//
	// Check whether we want to do a through check.
	//
	_thorough = properties->getPropertyAsInt("IcePatch.Thorough") > 0;

        //
        // Create and install the node description factory.
        //
        ObjectFactoryPtr factory = new FileDescFactory;
        communicator()->addObjectFactory(factory, "::IcePatch::DirectoryDesc");
        communicator()->addObjectFactory(factory, "::IcePatch::RegularDesc");

	//
	// Patch all subdirectories.
	//
	FilePrx top;
	for(vector<string>::const_iterator p = subdirs.begin(); p != subdirs.end(); ++p)
	{
	    Identity identity = pathToIdentity(*p);
	    ObjectPrx topObj = communicator()->stringToProxy(identityToString(identity) + ':' + endpoints);
	    
	    try
	    {
		top = FilePrx::checkedCast(topObj);
	    }
	    catch(const ObjectNotExistException&)
	    {
	    }

	    if(!top)
	    {
		cerr << appName() << ": `" << *p << "' does not exist" << endl;
		return EXIT_FAILURE;
	    }

	    DirectoryDescPtr topDesc = DirectoryDescPtr::dynamicCast(top->describe());
	    if(!topDesc)
	    {
		cerr << appName() << ": `" << *p << "' is not a directory" << endl;
		return EXIT_FAILURE;
	    }

	    string::size_type pos = 0;
	    while(pos < p->size())
	    {
		string::size_type pos2 = p->find('/', pos);
		if(pos2 == string::npos)
		{
		    pos2 = p->size();
		}
		string subPath = p->substr(0, pos2);
		FileInfo subPathInfo = getFileInfo(subPath, false);
		if(subPathInfo.type == FileTypeNotExist)
		{
		    createDirectory(subPath);
		    cout << subPath << ": created" << endl;
		}
		pos = pos2 + 1;		    
	    }

	    cout << pathToName(*p) << endl;
	    
	    patch(topDesc, "");
	}
    }
    catch(const FileAccessException& ex)
    {
	cout << endl; // There might still be a non-terminated line on cout.
	cerr << appName() << ": " << ex << ":\n" << ex.reason << endl;
	if(router)
	{
	    router->shutdown();
	}
	return EXIT_FAILURE;
    }
    catch(const BusyException&)
    {
	cout << endl; // There might still be a non-terminated line on cout.
	cerr << appName() << ": patching service busy, try again later" << endl;
	if(router)
	{
	    router->shutdown();
	}
	return EXIT_FAILURE;
    }
    catch(...)
    {
	cout << endl; // There might still be a non-terminated line on cout.
	if(router)
	{
	    router->shutdown();
	}
	throw; // Let Application::main() print the error message for this exception.
    }
    
    if(router)
    {
	router->shutdown();
    }

    return EXIT_SUCCESS;
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

#ifdef _WIN32

//
// Function object to do case-insensitive string comparison.
//
class CICompare : public std::binary_function<std::string, std::string, bool>
{
public:

    bool operator()(const string& s1, const string& s2) const
    {
	string::const_iterator p1 = s1.begin();
	string::const_iterator p2 = s2.begin();
	while(p1 != s1.end() && p2 != s2.end() && ::tolower(*p1) == ::tolower(*p2))
	{
	    ++p1;
	    ++p2;
	}
	if(p1 == s1.end() && p2 == s2.end())
	{
	    return false;
	}
	else if(p1 == s1.end())
	{
	    return true;
	}
	else if(p2 == s2.end())
	{
	    return false;
	}
	else
	{
	    return ::tolower(*p1) < ::tolower(*p2);
	}
    }
};

typedef set<string, CICompare> OrphanedSet;

#else

typedef set<string> OrphanedSet;

#endif

void
IcePatch::Client::patch(const DirectoryDescPtr& dirDesc, const string& indent) const
{
    OrphanedSet orphaned;

    if(_remove)
    {
	StringSeq fullDirectoryListing = readDirectory(identityToPath(dirDesc->dir->ice_getIdentity()));
	for(StringSeq::const_iterator p = fullDirectoryListing.begin(); p != fullDirectoryListing.end(); ++p)
	{
	    orphaned.insert(*p);
	}
    }
    
    FileDescSeq fileDescSeq = dirDesc->dir->getContents();

    for(unsigned int i = 0; i < fileDescSeq.size(); ++i)
    {
	string path;
	DirectoryDescPtr subDirDesc = DirectoryDescPtr::dynamicCast(fileDescSeq[i]);
	RegularDescPtr regDesc;
	if(subDirDesc)
	{
	    path = identityToPath(subDirDesc->dir->ice_getIdentity());
	}
	else
	{
	    regDesc = RegularDescPtr::dynamicCast(fileDescSeq[i]);
	    assert(regDesc);
	    path = identityToPath(regDesc->reg->ice_getIdentity());
	}

	if(_remove)
	{
	    orphaned.erase(path);
	    orphaned.erase(path + ".md5");
	}

	bool last = (i == fileDescSeq.size() - 1) && orphaned.empty();
	
	if(subDirDesc)
	{
	    cout << indent << "+-" << pathToName(path) << ":";

	    FileInfo info = getFileInfo(path, false);
	    switch(info.type)
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

		    if(!_thorough && !subDirDesc->md5.empty())
		    {
			ByteSeq md5;
			
			string pathMD5 = path + ".md5";
			FileInfo infoMD5 = getFileInfo(pathMD5, false);
			if(infoMD5.type == FileTypeRegular && infoMD5.time >= info.time)
			{
			    md5 = getMD5(path);
			}
			
			if(md5 == subDirDesc->md5)
			{
			    continue;
			}
		    }

		    break;
		}
		
		case FileTypeRegular:
		{
		    removeRecursive(path);
		    createDirectory(path);
		    cout << " created" << endl;
		    break;
		}
	    }

	    string newIndent;
	    if(last)
	    {
		newIndent = indent + "  ";
	    }
	    else
	    {
		newIndent = indent + "| ";
	    }

	    patch(subDirDesc, newIndent);

	    if(!subDirDesc->md5.empty())
	    {
		putMD5(path, subDirDesc->md5);
	    }
	}
	else
	{
	    assert(regDesc);
	    cout << indent << "+-" << pathToName(path) << ":";

	    MyProgressCB progressCB;

	    FileInfo info = getFileInfo(path, false);
	    switch(info.type)
	    {
		case FileTypeNotExist:
		{
		    orphaned.erase(path + ".bz2");
		    getRegular(regDesc->reg, progressCB);
		    break;
		}

		case FileTypeDirectory:
		{
		    removeRecursive(path);
		    orphaned.erase(path + ".bz2");
		    getRegular(regDesc->reg, progressCB);
		    break;
		}

		case FileTypeRegular:
		{
		    ByteSeq md5;
		    
		    string pathMD5 = path + ".md5";
		    FileInfo infoMD5 = getFileInfo(pathMD5, false);
		    if(infoMD5.type == FileTypeRegular && infoMD5.time >= info.time)
		    {
			md5 = getMD5(path);
		    }

		    if(md5 != regDesc->md5)
		    {
			removeRecursive(path);
			orphaned.erase(path + ".bz2");
			getRegular(regDesc->reg, progressCB);
		    }
		    else
		    {
			cout << " ok" << endl;
		    }

		    break;
		}
	    }
	}
    }

    for(OrphanedSet::const_iterator p = orphaned.begin(); p != orphaned.end(); ++p)
    {
	cout << indent << "+-" << pathToName(*p) << ": removing orphaned file" << endl;
	removeRecursive(*p);
    }
}

int
main(int argc, char* argv[])
{
    Client app;
    return app.main(argc, argv);
}
