// **********************************************************************
//
// Copyright (c) 2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <IcePatch2/FileServerI.h>
#include <IcePatch2/Util.h>
#include <fstream>

using namespace std;
using namespace Ice;
using namespace IcePatch2;

namespace IcePatch2
{

class Client : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);

private:

    void usage(const std::string&);
};

};

class DecompressorDestroyer
{
public:

    DecompressorDestroyer(DecompressorPtr decompressor) :
	_decompressor(decompressor)
    {
    }

    ~DecompressorDestroyer()
    {
	_decompressor->destroy();
	_decompressor->getThreadControl().join();
    }

private:

    const DecompressorPtr _decompressor;
};

int
IcePatch2::Client::run(int argc, char* argv[])
{
    bool thorough = false;
    bool dry = false;
    string dataDir;

    for(int i = 1; i < argc; ++i)
    {
	if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
	{
	    usage(argv[0]);
	    return EXIT_FAILURE;
	}
	else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
	{
	    cout << ICE_STRING_VERSION << endl;
	    return EXIT_FAILURE;
	}
	else if(strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--thorough") == 0)
	{
	    thorough = true;
	}
	else if(strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dry") == 0)
	{
	    dry = true;
	}
        else if(argv[i][0] == '-')
        {
            cerr << argv[0] << ": unknown option `" << argv[i] << "'" << endl;
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        else
        {
            if(dataDir.empty())
            {
                dataDir = argv[i];
            }
            else
            {
		cerr << argv[0] << ": too many arguments" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
            }
        }
    }

    PropertiesPtr properties = communicator()->getProperties();

    if(dataDir.empty())
    {
	string dataDir = properties->getProperty("IcePatch2.Directory");
	if(dataDir.empty())
	{
	    cerr << argv[0] << ": no data directory specified" << endl;
	    usage(argv[0]);
	    return EXIT_FAILURE;
	}
    }

    Int chunk = properties->getPropertyAsIntWithDefault("IcePatch2.ChunkSize", 100000);
    if(chunk < 1)
    {
	chunk = 1;
    }

    try
    {
	FileInfoSeq infoSeq;

#ifdef _WIN32
	char cwd[_MAX_PATH];
	if(_getcwd(cwd, _MAX_PATH) == NULL)
#else
	char cwd[PATH_MAX];
	if(getcwd(cwd, PATH_MAX) == NULL)
#endif
	{
	    throw string("cannot get the current directory: ") + strerror(errno);
	}
	
	dataDir = normalize(string(cwd) + '/' + dataDir);
	
	if(chdir(dataDir.c_str()) == -1)
	{
	    throw "cannot change directory to `" + dataDir + "': " + strerror(errno);
	}

	if(!thorough)
	{
	    try
	    {
		loadFileInfoSeq(dataDir, infoSeq);
	    }
	    catch(const string& ex)
	    {
		cout << "Cannot load file summary:\n" << ex << endl;
		string answer;
		do
		{
		    cout << "Do a thorough patch? (yes/no)" << endl;
		    cin >> answer;
		    transform(answer.begin(), answer.end(), answer.begin(), ::tolower);
		    if(answer == "no")
		    {
			return EXIT_SUCCESS;
		    }
		}
		while(answer != "yes");
		thorough = true;
	    }
	}

	if(thorough)
	{
	    getFileInfoSeq(".", infoSeq, false, false, false);
	    saveFileInfoSeq(dataDir, infoSeq);
	}

	const char* endpointsProperty = "IcePatch2.Endpoints";
	const string endpoints = properties->getProperty(endpointsProperty);
	if(endpoints.empty())
	{
	    cerr << argv[0] << ": property `" << endpointsProperty << "' is not set" << endl;
	    return EXIT_FAILURE;
	}

	const char* idProperty = "IcePatch2.Identity";
	const Identity id = stringToIdentity(properties->getPropertyWithDefault(idProperty, "IcePatch2/server"));

	ObjectPrx fileServerBase = communicator()->stringToProxy(identityToString(id) + ':' + endpoints);
	FileServerPrx fileServer = FileServerPrx::checkedCast(fileServerBase);
	if(!fileServer)
	{
	    cerr << argv[0] << ": proxy `" << identityToString(id) << ':' << endpoints << "' is not a file server."
		 << endl;
	    return EXIT_FAILURE;
	}

	FileTree0 tree0;
	getFileTree0(infoSeq, tree0);
	
	FileInfoSeq removeFiles;
	FileInfoSeq updateFiles;

	ByteSeq empty(20, 0);

	if(tree0.checksum != fileServer->getChecksum())
	{
	    string progress = "0%";
	    cout << "Getting list of files to patch: " << progress << flush;

	    ByteSeqSeq checksum0Seq = fileServer->getChecksum0Seq();
	    if(checksum0Seq.size() != 256)
	    {
		cerr << argv[0] << ": server returned illegal value" << endl;
		return EXIT_FAILURE;
	    }

	    for(int node0 = 0; node0 < 256; ++node0)
	    {
		if(tree0.nodes[node0].checksum != checksum0Seq[node0])
		{
		    FileInfoSeq fileSeq = fileServer->getFileInfo1Seq(node0);
		    
		    sort(fileSeq.begin(), fileSeq.end(), FileInfoLess());
		    fileSeq.erase(unique(fileSeq.begin(), fileSeq.end(), FileInfoEqual()), fileSeq.end());
		    
		    set_difference(tree0.nodes[node0].files.begin(),
				   tree0.nodes[node0].files.end(),
				   fileSeq.begin(),
				   fileSeq.end(),
				   back_inserter(removeFiles),
				   FileInfoLess());

		    set_difference(fileSeq.begin(),
				   fileSeq.end(),
				   tree0.nodes[node0].files.begin(),
				   tree0.nodes[node0].files.end(),
				   back_inserter(updateFiles),
				   FileInfoLess());
		}
		
		for(unsigned int i = 0; i < progress.size(); ++i)
		{
		    cout << '\b';
		}
		ostringstream s;
		s << (node0 + 1) * 100 / 256 << '%';
		progress = s.str();
		cout << progress << flush;
	    }
	}

	cout << endl;

	sort(removeFiles.begin(), removeFiles.end(), FileInfoLess());
	sort(updateFiles.begin(), updateFiles.end(), FileInfoLess());

	FileInfoSeq::const_iterator p;

	if(!removeFiles.empty())
	{
	    p = removeFiles.begin();
	    while(p != removeFiles.end())
	    {
		cout << "remove: " << getBasename(p->path) << endl;
		
		if(!dry)
		{
		    try
		    {
			removeRecursive(p->path);
		    }
		    catch(const string&)
		    {
		    }
		}
		
		string dir = p->path + '/';
		
		do
		{
		    ++p;
		}
		while(p != removeFiles.end() && p->path.size() > dir.size() &&
		      p->path.compare(0, dir.size(), dir) == 0);
	    }
	    
	    if(!dry)
	    {
		FileInfoSeq newInfoSeq;
		newInfoSeq.reserve(infoSeq.size());
		
		set_difference(infoSeq.begin(),
			       infoSeq.end(),
			       removeFiles.begin(),
			       removeFiles.end(),
			       back_inserter(newInfoSeq),
			       FileInfoLess());
		
		infoSeq.swap(newInfoSeq);
		
		saveFileInfoSeq(dataDir, infoSeq);
	    }
	}

	if(!updateFiles.empty())
	{
	    string pathLog = dataDir + ".log";
	    ofstream fileLog;

	    if(!dry)
	    {
		fileLog.open(pathLog.c_str());
		if(!fileLog)
		{
		    cerr << argv[0] << ": cannot open `" + pathLog + "' for writing: " + strerror(errno);
		    return EXIT_FAILURE;
		}
	    }
	    
	    Long total = 0;
	    Long updated = 0;
	    
	    for(p = updateFiles.begin(); p != updateFiles.end(); ++p)
	    {
		if(p->size > 0) // Regular, non-empty file?
		{
		    total += p->size;
		}
	    }
	    
	    DecompressorPtr decompressor = new Decompressor;
	    DecompressorDestroyer decompressorDestroyer(decompressor);
	    decompressor->start();
	    
	    for(p = updateFiles.begin(); p != updateFiles.end(); ++p)
	    {
		cout << "update: " << getBasename(p->path) << ' ' << flush;

		if(p->size < 0) // Directory?
		{
		    if(!dry)
		    {
			createDirectoryRecursive(p->path);
			fileLog << *p << endl;
		    }
		}
		else // Regular file.
		{
		    string pathBZ2 = p->path + ".bz2";
		    ofstream fileBZ2;

		    if(!dry)
		    {
			string dir = getDirname(pathBZ2);
			if(!dir.empty())
			{
			    createDirectoryRecursive(dir);
			}

			try
			{
			    removeRecursive(pathBZ2);
			}
			catch(...)
			{
			}

			fileBZ2.open(pathBZ2.c_str(), ios::binary);
			if(!fileBZ2)
			{
			    cerr << argv[0] << ": cannot open `" + pathBZ2 + "' for writing: " + strerror(errno);
			    return EXIT_FAILURE;
			}
		    }
		    
		    Int pos = 0;
		    string progress;
	
		    while(pos < p->size)
		    {
			ByteSeq bytes;

			try
			{
			    bytes = fileServer->getFileCompressed(p->path, pos, chunk);
			}
			catch(const FileAccessException& ex)
			{
			    cerr << argv[0] << ": server error for `" << p->path << "':" << ex.reason << endl;
			    return EXIT_FAILURE;
			}

			if(bytes.empty())
			{
			    cerr << argv[0] << ": size mismatch for `" << p->path << "'" << endl;
			    return EXIT_FAILURE;
			}

			if(!dry)
			{
			    fileBZ2.write(reinterpret_cast<char*>(&bytes[0]), bytes.size());

			    if(!fileBZ2)
			    {
				cerr << argv[0] << ": cannot write `" + pathBZ2 + "': " + strerror(errno);
				return EXIT_FAILURE;
			    }
			}

			pos += bytes.size();
			updated += bytes.size();

			for(unsigned int i = 0; i < progress.size(); ++i)
			{
			    cout << '\b';
			}
			ostringstream s;
			s << pos << '/' << p->size << " (" << updated << '/' << total << ')';
			progress = s.str();
			cout << progress << flush;
		    }

		    if(!dry)
		    {
			fileBZ2.close();
			fileLog << *p << endl;

			decompressor->checkForException();
			decompressor->add(p->path);
		    }
		}
	    
		cout << endl;
	    }

	    decompressor->destroy();
	    decompressor->getThreadControl().join();
	    decompressor->checkForException();

	    if(!dry)
	    {
		fileLog.close();

		FileInfoSeq newInfoSeq;
		newInfoSeq.reserve(infoSeq.size());
	    
		set_union(infoSeq.begin(),
			  infoSeq.end(),
			  updateFiles.begin(),
			  updateFiles.end(),
			  back_inserter(newInfoSeq),
			  FileInfoLess());
	    
		infoSeq.swap(newInfoSeq);
		
		saveFileInfoSeq(dataDir, infoSeq);
	    }
	}
    }
    catch(const string& ex)
    {
        cerr << argv[0] << ": " << ex << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void
IcePatch2::Client::usage(const string& appName)
{
    string options =
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.\n"
	"-t, --thorough       Recalculate all checksums.\n"
	"-d, --dry            Don't update, do a dry run only.";

    cerr << "Usage: " << appName << " [options] [DIR]" << endl;
    cerr << options << endl;
}

int
main(int argc, char* argv[])
{
    Client app;
    return app.main(argc, argv);
}
