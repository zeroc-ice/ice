// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <Ice/Application.h>
#include <Ice/SliceChecksums.h>
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

    void patch(const DirectoryDescPtr&, const string&, Long&, Long) const;

    bool _remove;
    bool _thorough;
    bool _dynamic;
};

}

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

    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    
    vector<string> subdirs;
    try
    {
    	subdirs = opts.parse(argc, argv);
    }
    catch(const IceUtil::Options::BadOpt& e)
    {
        cerr << e.reason << endl;
	usage();
	return EXIT_FAILURE;
    }

    if(opts.isSet("h") || opts.isSet("help"))
    {
	usage();
	return EXIT_SUCCESS;
    }
    if(opts.isSet("v") || opts.isSet("version"))
    {
	cout << ICE_STRING_VERSION << endl;
	return EXIT_SUCCESS;
    }

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

	if(subdirs.empty())
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

            //
            // Validate checksums of the Glacier Slice definitions.
            //
            Ice::SliceChecksumDict starterChecksums = starter->getSliceChecksums();
            Ice::SliceChecksumDict localChecksums = Ice::sliceChecksums();
            for(Ice::SliceChecksumDict::const_iterator q = starterChecksums.begin(); q != starterChecksums.end(); ++q)
            {
                Ice::SliceChecksumDict::const_iterator r = localChecksums.find(q->first);
                if(r == localChecksums.end())
                {
                    cerr << appName() << ": Glacier starter is using unknown Slice type `" << q->first << "'" << endl;
                }
                else if(q->second != r->second)
                {
                    cerr << appName() << ": Glacier starter is using a different Slice definition of `" << q->first
                         << "'" << endl;
                }
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
		catch(const Glacier::PermissionDeniedException&)
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

#ifdef _WIN32
        char cwd[_MAX_PATH];
        _getcwd(cwd, _MAX_PATH);
#else
        char cwd[PATH_MAX];
        getcwd(cwd, PATH_MAX);
#endif

	//
	// Check whether we want to remove orphaned files.
	//
	_remove = properties->getPropertyAsInt("IcePatch.RemoveOrphaned") > 0;
	if(_remove)
	{
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
	// Check whether we want to do a thorough check.
	//
	_thorough = properties->getPropertyAsInt("IcePatch.Thorough") > 0;

        //
        // Check whether we calculate MD5s for files dynamically.
        //
        _dynamic = properties->getPropertyAsIntWithDefault("IcePatch.Dynamic", 1) > 0;

        //
        // Create and install the node description factories.
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

            //
            // Validate Slice checksums. Note that the client contains more checksums
            // than the server because the client is linked with Glacier, therefore
            // we iterate over the server's checksums.
            //
            Ice::SliceChecksumDict serverChecksums = top->getSliceChecksums();
            Ice::SliceChecksumDict localChecksums = Ice::sliceChecksums();
            for(Ice::SliceChecksumDict::const_iterator q = serverChecksums.begin(); q != serverChecksums.end(); ++q)
            {
                Ice::SliceChecksumDict::const_iterator r = localChecksums.find(q->first);
                if(r == localChecksums.end())
                {
                    cerr << appName() << ": server is using unknown Slice type `" << q->first << "'" << endl;
                }
                else if(q->second != r->second)
                {
                    cerr << appName() << ": server is using a different Slice definition of `" << q->first << "'"
                         << endl;
                }
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

	    cout << pathToName(*p) << flush;

            string dir = *p;
            if(dir == ".")
            {
                dir = cwd;
            }

            ByteSeq md5;
            try
            {
                md5 = getMD5(dir);
            }
            catch(const FileAccessException&)
            {
                // MD5 file doesn't exist.
            }

            if(md5.empty())
            {
                //
                // Check the local directory before patching. This recursively
                // updates the MD5 files, including the creation (if necessary)
                // of an MD5 file for the top-level local directory. We need this
                // top-level MD5 for two purposes:
                //
                // 1. We can compare it with the MD5 of the server's directory, and
                //    skip the patch altogether if they match.
                //
                // 2. We may be able to obtain a download total by passing the MD5
                //    to the getTotal operation.
                //
                // If the top-level MD5 file did not exist, there are two likely
                // scenarios:
                //
                // 1. The local directory is new, in which case the entire server
                //    tree must be downloaded.
                //
                // 2. A previous patch was interrupted (note that we remove a
                //    directory's MD5 prior to patching it).
                //
                // In either case, we calculate a new MD5 for the directory, which
                // gives us the desired behavior:
                //
                // * If the local directory is empty, the MD5 will consist of all zeros,
                //   for which getTotal returns the download total for the server's
                //   current tree.
                //
                // * If the local directory is not empty, we get an accurate signature
                //   that will result in getTotal returning either a valid download total,
                //   or a value of -1 indicating some intermediate state (e.g., the patch
                //   was interrupted and is being resumed). In the case of a resumed
                //   patch, we can't accurately provide progress feedback, so a return
                //   value of -1 correctly causes the feedback to be suppressed.
                //
                checkDirectory(dir, _dynamic);
                md5 = getMD5(dir);
            }

            if(!_thorough && md5 == topDesc->md5)
            {
                //
                // Skip the directory if the MD5s match.
                //
                cout << ": ok" << endl;
            }
            else
            {
                cout << endl;

                //
                // Get the download total.
                //
                Long total = topDesc->dir->getTotal(md5);

                //
                // Remove the existing MD5 file prior to patching. If this patch
                // session is interrupted, checkDirectory will create a new MD5
                // file upon restart.
                //
                removeRecursive(dir + ".md5");

                //
                // Patch the directory.
                //
                Long runningTotal = 0;
                patch(topDesc, "", runningTotal, total);

                //
                // Create a new MD5 file for the directory.
                //
                createMD5(dir, _dynamic);
            }
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

    MyProgressCB(Long runningTotal, Long patchTotal) :
        _runningTotal(runningTotal), _patchTotal(patchTotal)
    {
    }

    virtual void startDownload(Int total, Int pos)
    {
	Ice::Int percent = pos * 100 / total;
	cout << " download " << setw(3) << percent << "%" << flush;
        _fileTotal = 0;
    }

    virtual void updateDownload(Int total, Int pos)
    {
	Ice::Int percent = pos * 100 / total;
	cout << "\b\b\b\b" << setw(3) << percent << "%" << flush;
    }

    virtual void finishedDownload(Int total)
    {
	updateDownload(total, total);
        _fileTotal = total;
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
        if(_patchTotal > 0)
        {
            Long percent = ((_runningTotal + _fileTotal) * 100) / _patchTotal;
            if(percent > 100)
            {
                percent = 100;
            }
            cout << " (" << static_cast<Int>(percent) << "% complete)";
        }
	cout << endl;
    }

private:

    Long _runningTotal;
    Long _patchTotal;
    Int _fileTotal;
};

#ifdef _WIN32

typedef set<string, CICompare> OrphanedSet;

#else

typedef set<string> OrphanedSet;

#endif

void
IcePatch::Client::patch(const DirectoryDescPtr& dirDesc, const string& indent,
			Long& runningTotal, Long patchTotal) const
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
                            cout << " ok" << endl;
			    continue;
			}
		    }

                    cout << endl;

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

            //
            // Remove any existing MD5 file prior to patching the subdirectory.
            // If we are interrupted before this subdirectory is complete patched,
            // a new MD5 is calculated upon resumption.
            //
            try
            {
                removeRecursive(path + ".md5");
            }
            catch(const FileAccessException&)
            {
            }

	    patch(subDirDesc, newIndent, runningTotal, patchTotal);

	    if(!subDirDesc->md5.empty())
	    {
		putMD5(path, subDirDesc->md5);
	    }
	}
	else
	{
	    assert(regDesc);
	    cout << indent << "+-" << pathToName(path) << ":";

	    MyProgressCB progressCB(runningTotal, patchTotal);

	    bool update = false;

	    FileInfo info = getFileInfo(path, false);
            string pathMD5 = path + ".md5";
            FileInfo infoMD5 = getFileInfo(pathMD5, false);

	    switch(info.type)
	    {
		case FileTypeNotExist:
		{
		    update = true;
		    break;
		}

		case FileTypeDirectory:
		{
		    removeRecursive(path);
		    update = true;
		    break;
		}

		case FileTypeRegular:
		{
		    ByteSeq md5;
		    
		    if(_thorough || _dynamic)
		    {
			md5 = calcMD5(path, _dynamic);
		    }
		    else
		    {
			if(infoMD5.type == FileTypeRegular && infoMD5.time >= info.time)
			{
			    md5 = getMD5(path);
			}
		    }

		    if(md5 != regDesc->md5)
		    {
			removeRecursive(path);
			update = true;
		    }
		    else
		    {
			cout << " ok" << endl;

                        //
                        // Cache the MD5 value if none is present.
                        //
                        if(!_dynamic && infoMD5.type == FileTypeNotExist)
                        {
                            putMD5(path, md5);
                        }
		    }

		    break;
		}
	    }

            //
            // We need to remove the existing MD5 file if we are calculating
            // MD5s dynamically, or if we are about to update the file.
            //
            if(infoMD5.type != FileTypeNotExist && (_dynamic || update))
            {
                removeRecursive(pathMD5);
            }

	    if(update)
	    {
		orphaned.erase(path + ".bz2");

		int retries = 0;
		while(true)
		{
		    //
		    // We don't want to use compression when we
		    // retrieve files, since files are pre-compressed
		    // already. (Disabling compression here is only
		    // relevant if a router is used. Otherwise, the
		    // icepatch server already gives us a proxy for
		    // the regular file with compression disabled.)
		    //
		    RegularPrx regular = RegularPrx::uncheckedCast(regDesc->reg->ice_compress(false));

		    //
		    // Retrieve file from server.
		    //
		    ByteSeq md5 = getRegular(regular, progressCB);

		    //
		    // Get the latest file description from server, as
		    // the file may have recently changed.
		    //
		    regDesc = RegularDescPtr::dynamicCast(regDesc->reg->describe());
		    if(!regDesc)
		    {
			removeRecursive(path);

			FileAccessException ex;
			ex.reason = "Unexpected error accessing `" + path + "' remotely.";
			throw ex;
		    }

		    //
		    // Compare the MD5s of the server and the local file.
		    // If they differ, try again unless we've hit the
		    // retry limit.
		    //
		    if(regDesc->md5 == md5)
		    {
                        runningTotal += regDesc->reg->getBZ2Size();
                        if(!_dynamic)
                        {
                            putMD5(path, md5);
                        }
			break;
		    }
		    else if(retries < 3)
		    {
			++retries;
		    }
		    else
		    {
			removeRecursive(path);

			FileAccessException ex;
			ex.reason = "Retry count exceeded while patching `" + path + "':\nMD5 mismatch";
			throw ex;
		    }
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
