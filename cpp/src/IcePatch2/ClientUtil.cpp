// **********************************************************************
//
// Copyright (c) 2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IcePatch2/ClientUtil.h>
#include <IcePatch2/Util.h>
#include <IcePatch2/FileServerI.h>

#ifdef _WIN32
#   include <direct.h>
#endif

using namespace std;
using namespace Ice;
using namespace IcePatch2;

IcePatch2::Patcher::Patcher(const CommunicatorPtr& communicator, const PatcherFeedbackPtr& feedback) :
    _communicator(communicator),
    _feedback(feedback),
    _stop(false)
{
}

IcePatch2::Patcher::~Patcher()
{
    assert(_stop);
}

void
IcePatch2::Patcher::patch()
{
    PropertiesPtr properties = _communicator->getProperties();

    string dataDir = properties->getProperty("IcePatch2.Directory");
    bool dryRun = properties->getPropertyAsInt("IcePatch2.DryRun") > 0;
    bool thorough = properties->getPropertyAsInt("IcePatch2.Thorough") > 0;
    Int chunkSize = properties->getPropertyAsIntWithDefault("IcePatch2.ChunkSize", 100000);

    if(dataDir.empty())
    {
	throw "no data directory specified";
    }

    if(chunkSize < 1)
    {
	chunkSize = 1;
    }

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
	    thorough = _feedback->noFileSummary(ex);
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
	throw "property `" + endpointsProperty + "' is not set";
    }
    
    const char* idProperty = "IcePatch2.Identity";
    const Identity id = stringToIdentity(properties->getPropertyWithDefault(idProperty, "IcePatch2/server"));
    
    ObjectPrx fileServerBase = _communicator->stringToProxy(identityToString(id) + ':' + endpoints);
    FileServerPrx fileServer = FileServerPrx::checkedCast(fileServerBase);
    if(!fileServer)
    {
	throw "proxy `" + identityToString(id) + ':' + endpoints + "' is not a file server.";
    }
    
    FileTree0 tree0;
    getFileTree0(infoSeq, tree0);
    
    FileInfoSeq removeFiles;
    FileInfoSeq updateFiles;
    
    ByteSeq empty(20, 0);
    
    if(tree0.checksum != fileServer->getChecksum())
    {
	_feedback->fileListStart();
	
	ByteSeqSeq checksum0Seq = fileServer->getChecksum0Seq();
	if(checksum0Seq.size() != 256)
	{
	    throw "server returned illegal value";
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

	    _feedback->fileListProgress((node0 + 1) * 100 / 256);
	}

	_feedback->fileListEnd();
    }
    
    sort(removeFiles.begin(), removeFiles.end(), FileInfoLess());
    sort(updateFiles.begin(), updateFiles.end(), FileInfoLess());
    
    FileInfoSeq::const_iterator p;
    
    if(!removeFiles.empty())
    {
	p = removeFiles.begin();
	while(p != removeFiles.end())
	{
	    if(!dryRun)
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
	
	if(!dryRun)
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
    
    try
    {
	if(!updateFiles.empty())
	{
	    Long total = 0;
	    Long updated = 0;
	
	    for(p = updateFiles.begin(); p != updateFiles.end(); ++p)
	    {
		if(p->size > 0) // Regular, non-empty file?
		{
		    total += p->size;
		}
	    }
	
	    if(!dryRun)
	    {
		string pathLog = dataDir + ".log";
		_fileLog.open(pathLog.c_str());
		if(!_fileLog)
		{
		    throw "cannot open `" + pathLog + "' for writing: " + strerror(errno);
		}

		start();
	    }

	    for(p = updateFiles.begin(); p != updateFiles.end(); ++p)
	    {
		if(p->size < 0) // Directory?
		{
		    if(!dryRun)
		    {
			createDirectoryRecursive(p->path);

			{
			    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
			    _fileLog << *p << endl;
			}
		    }
		}
		else // Regular file.
		{
		    string pathBZ2 = p->path + ".bz2";
		    ofstream fileBZ2;

		    if(!dryRun)
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
			    throw "cannot open `" + pathBZ2 + "' for writing: " + strerror(errno);
			}

			_feedback->patchStart(p->path, p->size, updated, total);
		    }
		    
		    Int pos = 0;
		    string progress;
	
		    while(pos < p->size)
		    {
			ByteSeq bytes;

			try
			{
			    bytes = fileServer->getFileCompressed(p->path, pos, chunkSize);
			}
			catch(const FileAccessException& ex)
			{
			    throw "server error for `" + p->path + "':" + ex.reason;
			}

			if(bytes.empty())
			{
			    throw "size mismatch for `" + p->path + "'";
			}

			if(!dryRun)
			{
			    fileBZ2.write(reinterpret_cast<char*>(&bytes[0]), bytes.size());

			    if(!fileBZ2)
			    {
				throw ": cannot write `" + pathBZ2 + "': " + strerror(errno);
			    }
			}

			pos += bytes.size();
			updated += bytes.size();

			_feedback->patchProgress(pos, p->size, updated, total);
		    }

		    if(!dryRun)
		    {
			fileBZ2.close();

			{
			    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

			    if(!_exception.empty())
			    {
				throw _exception;
			    }

			    _infoSeq.push_back(*p);
			    notify();
			}
		    }
		}
	    
		_feedback->patchEnd();
	    }

	    if(!dryRun)
	    {
		{
		    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
		
		    if(!_exception.empty())
		    {
			throw _exception;
		    }

		    _stop = true;
		    notify();
		}

		getThreadControl().join();

		_fileLog.close();

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
    catch(...)
    {
	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	    _stop = true;
	    notify();
	}
	
	getThreadControl().join();

	throw;
    }
}

void
IcePatch2::Patcher::run()
{
    FileInfo info;
    
    while(true)
    {
	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	    
	    if(!info.path.empty())
	    {
		_fileLog << info << endl;
	    }
	    
	    while(!_stop && _infoSeq.empty())
	    {
		wait();
	    }
	    
	    if(!_infoSeq.empty())
	    {
		info = _infoSeq.front();
		_infoSeq.pop_front();
	    }
	    else
	    {
		assert(_stop);
		return;
	    }
	}
	
	try
	{
	    decompressFile(info.path);
	    remove(info.path + ".bz2");
	}
	catch(const string& ex)
	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	    _stop = true;
	    _exception = ex;
	    return;
	}
    }
}
