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
    _feedback(feedback),
    _dataDir(normalize(communicator->getProperties()->getProperty("IcePatch2.Directory"))),
    _dryRun(communicator->getProperties()->getPropertyAsInt("IcePatch2.DryRun") > 0),
    _thorough(communicator->getProperties()->getPropertyAsInt("IcePatch2.Thorough") > 0),
    _chunkSize(communicator->getProperties()->getPropertyAsIntWithDefault("IcePatch2.ChunkSize", 100000)),
    _decompress(false)
{
    if(_dataDir.empty())
    {
	throw string("no data directory specified");
    }

    if(_chunkSize < 1)
    {
	const_cast<Int&>(_chunkSize) = 1;
    }

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
    
    const_cast<string&>(_dataDir) = normalize(string(cwd) + '/' + _dataDir);
    
    PropertiesPtr properties = communicator->getProperties();

    const char* endpointsProperty = "IcePatch2.Endpoints";
    const string endpoints = properties->getProperty(endpointsProperty);
    if(endpoints.empty())
    {
	throw string("property `") + endpointsProperty + "' is not set";
    }
    
    const char* idProperty = "IcePatch2.Identity";
    const Identity id = stringToIdentity(properties->getPropertyWithDefault(idProperty, "IcePatch2/server"));
    
    ObjectPrx serverBase = communicator->stringToProxy(identityToString(id) + ':' + endpoints);
    const_cast<FileServerPrx&>(_serverCompress) = FileServerPrx::checkedCast(serverBase->ice_compress(true));
    if(!_serverCompress)
    {
	throw "proxy `" + identityToString(id) + ':' + endpoints + "' is not a file server.";
    }
    const_cast<FileServerPrx&>(_serverNoCompress) = FileServerPrx::checkedCast(_serverCompress->ice_compress(false));
}

IcePatch2::Patcher::~Patcher()
{
    assert(!_decompress);
}

void
IcePatch2::Patcher::patch()
{
    FileInfoSeq infoSeq;
   
    bool thorough = _thorough;

    if(!thorough)
    {
	try
	{
	    loadFileInfoSeq(_dataDir, infoSeq);
	}
	catch(const string& ex)
	{
	    thorough = _feedback->noFileSummary(ex);
	    if(!thorough)
	    {
		return;
	    }
	}
    }
    
    if(thorough)
    {
	getFileInfoSeq(_dataDir, infoSeq, false, false, false);
	saveFileInfoSeq(_dataDir, infoSeq);
    }
        
    FileTree0 tree0;
    getFileTree0(infoSeq, tree0);
    
    FileInfoSeq removeFileSeq;
    FileInfoSeq updateFileSeq;
    
    ByteSeq empty(20, 0);

    if(tree0.checksum != _serverCompress->getChecksum())
    {
	_feedback->fileListStart();
	
	ByteSeqSeq checksum0Seq = _serverCompress->getChecksum0Seq();
	if(checksum0Seq.size() != 256)
	{
	    throw string("server returned illegal value");
	}
	
	for(int node0 = 0; node0 < 256; ++node0)
	{
	    if(tree0.nodes[node0].checksum != checksum0Seq[node0])
	    {
		FileInfoSeq fileSeq = _serverCompress->getFileInfo1Seq(node0);
		
		sort(fileSeq.begin(), fileSeq.end(), FileInfoLess());
		fileSeq.erase(unique(fileSeq.begin(), fileSeq.end(), FileInfoEqual()), fileSeq.end());
		
		set_difference(tree0.nodes[node0].files.begin(),
			       tree0.nodes[node0].files.end(),
			       fileSeq.begin(),
			       fileSeq.end(),
			       back_inserter(removeFileSeq),
			       FileInfoLess());
		
		set_difference(fileSeq.begin(),
			       fileSeq.end(),
			       tree0.nodes[node0].files.begin(),
			       tree0.nodes[node0].files.end(),
			       back_inserter(updateFileSeq),
			       FileInfoLess());
	    }

	    _feedback->fileListProgress((node0 + 1) * 100 / 256);
	}

	_feedback->fileListEnd();
    }
    
    sort(removeFileSeq.begin(), removeFileSeq.end(), FileInfoLess());
    sort(updateFileSeq.begin(), updateFileSeq.end(), FileInfoLess());
    
    if(!removeFileSeq.empty())
    {
	removeFiles(removeFileSeq);

	if(!_dryRun)
	{
	    FileInfoSeq newInfoSeq;
	    newInfoSeq.reserve(infoSeq.size());
	    
	    set_difference(infoSeq.begin(),
			   infoSeq.end(),
			   removeFileSeq.begin(),
			   removeFileSeq.end(),
			   back_inserter(newInfoSeq),
			   FileInfoLess());
	    
	    infoSeq.swap(newInfoSeq);
	    
	    saveFileInfoSeq(_dataDir, infoSeq);
	}
    }

    if(!updateFileSeq.empty())
    {
	if(!_dryRun)
	{
	    string pathLog = _dataDir + ".log";
	    _fileLog.open(pathLog.c_str());
	    if(!_fileLog)
	    {
		throw "cannot open `" + pathLog + "' for writing: " + strerror(errno);
	    }
	    
	    {
		IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
		_decompress = true;
	    }

	    start();
	}
	
	try
	{
	    updateFiles(updateFileSeq);
	}
	catch(...)
	{
	    {
		IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
		_decompress = false;
		notify();
	    }

	    getThreadControl().join();

	    throw;
	}

	if(!_dryRun)
	{
	    {
		IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
		if(!_decompressException.empty())
		{
		    throw _decompressException;
		}
		_decompress = false;
		notify();
	    }

	    getThreadControl().join();
	    
	    _fileLog.close();
	    
	    FileInfoSeq newInfoSeq;
	    newInfoSeq.reserve(infoSeq.size());
	    
	    set_union(infoSeq.begin(),
		      infoSeq.end(),
		      updateFileSeq.begin(),
		      updateFileSeq.end(),
		      back_inserter(newInfoSeq),
		      FileInfoLess());
	    
	    infoSeq.swap(newInfoSeq);
	    
	    saveFileInfoSeq(_dataDir, infoSeq);
	}
    }
}

void
IcePatch2::Patcher::removeFiles(const FileInfoSeq& files)
{
    if(!_dryRun)
    {
	FileInfoSeq::const_iterator p = files.begin();
	
	while(p != files.end())
	{
	    removeRecursive(_dataDir + '/' + p->path);
	    
	    string dir = p->path + '/';
	    
	    do
	    {
		++p;
	    }
	    while(p != files.end() && p->path.size() > dir.size() &&
		  p->path.compare(0, dir.size(), dir) == 0);
	}
    }
}
    
void
IcePatch2::Patcher::updateFiles(const FileInfoSeq& files)
{
    FileInfoSeq::const_iterator p;
    
    Long total = 0;
    Long updated = 0;
    
    for(p = files.begin(); p != files.end(); ++p)
    {
	if(p->size > 0) // Regular, non-empty file?
	{
	    total += p->size;
	}
    }
    
    for(p = files.begin(); p != files.end(); ++p)
    {
	if(p->size < 0) // Directory?
	{
	    if(!_dryRun)
	    {
		createDirectoryRecursive(_dataDir + '/' + p->path);
		
		{
		    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
		    _fileLog << *p << endl;
		}
	    }
	}
	else // Regular file.
	{
	    _feedback->patchStart(p->path, p->size, updated, total);

	    string pathBZ2 = _dataDir + '/' + p->path + ".bz2";
	    ofstream fileBZ2;
	    
	    if(!_dryRun)
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
	    }
	    
	    Int pos = 0;
	    string progress;
	    
	    while(pos < p->size)
	    {
		ByteSeq bytes;
		
		try
		{
		    bytes = _serverNoCompress->getFileCompressed(p->path, pos, _chunkSize);
		}
		catch(const FileAccessException& ex)
		{
		    throw "server error for `" + p->path + "':" + ex.reason;
		}
		
		if(bytes.empty())
		{
		    throw "size mismatch for `" + p->path + "'";
		}
		
		if(!_dryRun)
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
	    
	    if(!_dryRun)
	    {
		fileBZ2.close();
		
		{
		    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
		    if(!_decompressException.empty())
		    {
			throw _decompressException;
		    }
		    _decompressList.push_back(*p);
		    notify();
		}
	    }
	
	    _feedback->patchEnd();
	}
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
	    
	    while(_decompress && _decompressList.empty())
	    {
		wait();
	    }
	    
	    if(!_decompressList.empty())
	    {
		info = _decompressList.front();
		_decompressList.pop_front();
	    }
	    else
	    {
		return;
	    }
	}
	
	try
	{
	    decompressFile(_dataDir + '/' + info.path);
	    remove(_dataDir + '/' + info.path + ".bz2");
	}
	catch(const string& ex)
	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	    _decompress = false;
	    _decompressException = ex;
	    return;
	}
    }
}
