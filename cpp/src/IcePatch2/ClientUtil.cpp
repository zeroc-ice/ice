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
#include <list>

#ifdef _WIN32
#   include <direct.h>
#endif

using namespace std;
using namespace Ice;
using namespace IcePatch2;

namespace IcePatch2
{

class Decompressor : public IceUtil::Thread, IceUtil::Monitor<IceUtil::Mutex>
{
public:

    Decompressor(const string& dataDir) :
	_dataDir(dataDir),
	_destroy(false)
    {
    }

    virtual ~Decompressor()
    {
	assert(_destroy);
    }

    void
    destroy()
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	_destroy = true;
	notify();
    }

    void
    add(const FileInfo info)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	if(!_exception.empty())
	{
	    throw _exception;
	}
	_files.push_back(info);
	notify();
    }

    void
    exception() const
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	if(!_exception.empty())
	{
	    throw _exception;
	}
    }

    void
    log(ofstream& os)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	for(FileInfoSeq::const_iterator p = _filesDone.begin(); p != _filesDone.end(); ++p)
	{
	    os << '+' << *p << endl;
	}

	_filesDone.clear();
    }

    virtual void
    run()
    {
	FileInfo info;
	
	while(true)
	{
	    {
		IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	    
		if(!info.path.empty())
		{
		    _filesDone.push_back(info);
		}
	    
		while(!_destroy && _files.empty())
		{
		    wait();
		}
	    
		if(!_files.empty())
		{
		    info = _files.front();
		    _files.pop_front();
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
		_destroy = true;
		_exception = ex;
		return;
	    }
	}
    }

private:

    const string _dataDir;

    string _exception;
    list<FileInfo> _files;
    FileInfoSeq _filesDone;

    bool _destroy;
};

}

IcePatch2::Patcher::Patcher(const CommunicatorPtr& communicator, const PatcherFeedbackPtr& feedback) :
    _feedback(feedback),
    _dataDir(normalize(communicator->getProperties()->getProperty("IcePatch2.Directory"))),
    _thorough(communicator->getProperties()->getPropertyAsInt("IcePatch2.Thorough") > 0),
    _chunkSize(communicator->getProperties()->getPropertyAsIntWithDefault("IcePatch2.ChunkSize", 100000)),
    _remove(communicator->getProperties()->getPropertyAsIntWithDefault("IcePatch2.Remove", 1))
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
    if(_dataDir[0] != '/' && !(_dataDir.size() > 1 && isalpha(_dataDir[0]) && _dataDir[1] == ':'))
    {
	char cwd[_MAX_PATH];
	if(_getcwd(cwd, _MAX_PATH) == NULL)
#else
    if(_dataDir[0] != '/')
    {
	char cwd[PATH_MAX];
	if(getcwd(cwd, PATH_MAX) == NULL)
#endif
	{
	    throw "cannot get the current directory:\n" + lastError();
	}
    
	const_cast<string&>(_dataDir) = string(cwd) + '/' + _dataDir;
    }
    
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
}

class PatcherGetFileInfoSeqCB : public GetFileInfoSeqCB
{
public:

    PatcherGetFileInfoSeqCB(const PatcherFeedbackPtr& feedback) :
	_feedback(feedback)
    {
    }

    virtual bool
    remove(const string&)
    {
	return true;
    }

    virtual bool
    checksum(const string& path)
    {
	return _feedback->checksumProgress(path);
    }

    virtual bool compress(const string&)
    {
	assert(false); // Nothing must get compressed when we are patching.
	return true;
    }

private:
    
    const PatcherFeedbackPtr _feedback;
};

bool
IcePatch2::Patcher::prepare()
{
    _localFiles.clear();

    bool thorough = _thorough;

    if(!thorough)
    {
	try
	{
	    loadFileInfoSeq(_dataDir, _localFiles);
	}
	catch(const string& ex)
	{
	    thorough = _feedback->noFileSummary(ex);
	    if(!thorough)
	    {
		return false;
	    }
	}
    }
    
    if(thorough)
    {
	if(!_feedback->checksumStart())
	{
	    return false;
	}

	PatcherGetFileInfoSeqCB cb(_feedback);
	if(!getFileInfoSeq(_dataDir, 0, &cb, _localFiles))
	{
	    return false;
	}

	if(!_feedback->checksumEnd())
	{
	    return false;	   
	}

	saveFileInfoSeq(_dataDir, _localFiles);
    }

    FileTree0 tree0;
    getFileTree0(_localFiles, tree0);

    if(tree0.checksum != _serverCompress->getChecksum())
    {
	if(!_feedback->fileListStart())
	{
	    return false;
	}
	
	ByteSeqSeq checksum0Seq = _serverCompress->getChecksum0Seq();
	if(checksum0Seq.size() != 256)
	{
	    throw string("server returned illegal value");
	}
	
	for(int node0 = 0; node0 < 256; ++node0)
	{
	    if(tree0.nodes[node0].checksum != checksum0Seq[node0])
	    {
		FileInfoSeq files = _serverCompress->getFileInfo1Seq(node0);
		
		sort(files.begin(), files.end(), FileInfoLess());
		files.erase(unique(files.begin(), files.end(), FileInfoEqual()), files.end());
		
		set_difference(tree0.nodes[node0].files.begin(),
			       tree0.nodes[node0].files.end(),
			       files.begin(),
			       files.end(),
			       back_inserter(_removeFiles),
			       FileInfoLess());
		
		set_difference(files.begin(),
			       files.end(),
			       tree0.nodes[node0].files.begin(),
			       tree0.nodes[node0].files.end(),
			       back_inserter(_updateFiles),
			       FileInfoLess());
	    }

	    if(!_feedback->fileListProgress((node0 + 1) * 100 / 256))
	    {
		return false;
	    }
	}

	if(!_feedback->fileListEnd())
	{
	    return false;
	}
    }
    
    sort(_removeFiles.begin(), _removeFiles.end(), FileInfoLess());
    sort(_updateFiles.begin(), _updateFiles.end(), FileInfoLess());

    string pathLog = _dataDir + ".log";
    _log.open(pathLog.c_str());
    if(!_log)
    {
	throw "cannot open `" + pathLog + "' for writing:\n" + lastError();
    }

    return true;
}

bool
IcePatch2::Patcher::patch(const string& d)
{
    string dir = normalize(d);

    if(dir.empty() || dir == ".")
    {
	if(!_removeFiles.empty())
	{
	    if(!removeFiles(_removeFiles))
	    {
		return false;
	    }
	}
	
	if(!_updateFiles.empty())
	{
	    if(!updateFiles(_updateFiles))
	    {
		return false;
	    }
	}
	
	return true;
    }
    else
    {
	string dirWithSlash = dir + '/';

	FileInfoSeq::const_iterator p;

	FileInfoSeq remove;
	for(p = _removeFiles.begin(); p != _removeFiles.end(); ++p)
	{
	    if(p->path == dir)
	    {
		remove.push_back(*p);
	    }
	    else if(p->path.compare(0, dirWithSlash.size(), dirWithSlash) == 0)
	    {
		remove.push_back(*p);
	    }
	}

	FileInfoSeq update;
	for(p = _updateFiles.begin(); p != _updateFiles.end(); ++p)
	{
	    if(p->path == dir)
	    {
		update.push_back(*p);
	    }
	    else if(p->path.compare(0, dirWithSlash.size(), dirWithSlash) == 0)
	    {
		update.push_back(*p);
	    }
	}

	if(!remove.empty())
	{
	    if(!removeFiles(remove))
	    {
		return false;
	    }
	}
	
	if(!update.empty())
	{
	    if(!updateFiles(update))
	    {
		return false;
	    }
	}
	
	return true;
    }
}

void
IcePatch2::Patcher::finish()
{
    _log.close();

    saveFileInfoSeq(_dataDir, _localFiles);
}

bool
IcePatch2::Patcher::removeFiles(const FileInfoSeq& files)
{
    if(_remove < 1)
    {
	return true;
    }

    for(FileInfoSeq::const_reverse_iterator p = files.rbegin(); p != files.rend(); ++p)
    {
	try
	{
	    remove(_dataDir + '/' + p->path);
	    _log << '-' << *p << endl;
	}
	catch(...)
	{
	    if(_remove < 2) // We ignore errors if IcePatch2.Remove >= 2.
	    {
		throw;
	    }
	}
    }
    
    FileInfoSeq newLocalFiles;
    newLocalFiles.reserve(_localFiles.size());
    
    set_difference(_localFiles.begin(),
		   _localFiles.end(),
		   files.begin(),
		   files.end(),
		   back_inserter(newLocalFiles),
		   FileInfoLess());
    
    _localFiles.swap(newLocalFiles);
    
    FileInfoSeq newRemoveFiles;
    
    set_difference(_removeFiles.begin(),
		   _removeFiles.end(),
		   files.begin(),
		   files.end(),
		   back_inserter(newRemoveFiles),
		   FileInfoLess());
    
    _removeFiles.swap(newRemoveFiles);

    return true;
}
    
bool
IcePatch2::Patcher::updateFiles(const FileInfoSeq& files)
{
    DecompressorPtr decompressor = new Decompressor(_dataDir);
    decompressor->start();

    bool result;

    try
    {
	result = updateFilesInternal(files, decompressor);
    }
    catch(...)
    {
	decompressor->destroy();
	decompressor->getThreadControl().join();
	decompressor->log(_log);
	throw;
    }
    
    decompressor->destroy();
    decompressor->getThreadControl().join();
    decompressor->log(_log);
    decompressor->exception();

    return result;
}

bool
IcePatch2::Patcher::updateFilesInternal(const FileInfoSeq& files, const DecompressorPtr& decompressor)
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
	    createDirectoryRecursive(_dataDir + '/' + p->path);
	    _log << '+' << *p << endl;
	}
	else // Regular file.
	{
	    if(!_feedback->patchStart(p->path, p->size, updated, total))
	    {
		return false;
	    }

	    if(p->size == 0)
	    {
		string path = _dataDir + '/' + p->path;
		ofstream file(path.c_str(), ios::binary);
	    }
	    else
	    {
		string pathBZ2 = _dataDir + '/' + p->path + ".bz2";
	    
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
		
		ofstream fileBZ2(pathBZ2.c_str(), ios::binary);
		if(!fileBZ2)
		{
		    throw "cannot open `" + pathBZ2 + "' for writing:\n" + lastError();
		}
	    
		Int pos = 0;
	    
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
		
		    fileBZ2.write(reinterpret_cast<char*>(&bytes[0]), bytes.size());
		
		    if(!fileBZ2)
		    {
			throw ": cannot write `" + pathBZ2 + "':\n" + lastError();
		    }
		
		    pos += bytes.size();
		    updated += bytes.size();
		
		    if(!_feedback->patchProgress(pos, p->size, updated, total))
		    {
			return false;
		    }
		}
	    
		fileBZ2.close();

		decompressor->log(_log);
		decompressor->add(*p);
	    }
	
	    if(!_feedback->patchEnd())
	    {
		return false;
	    }
	}
    }

    FileInfoSeq newLocalFiles;
    newLocalFiles.reserve(_localFiles.size());
	
    set_union(_localFiles.begin(),
	      _localFiles.end(),
	      files.begin(),
	      files.end(),
	      back_inserter(newLocalFiles),
	      FileInfoLess());
	
    _localFiles.swap(newLocalFiles);

    FileInfoSeq newUpdateFiles;

    set_difference(_updateFiles.begin(),
		   _updateFiles.end(),
		   files.begin(),
		   files.end(),
		   back_inserter(newUpdateFiles),
		   FileInfoLess());
	
    _updateFiles.swap(newUpdateFiles);

    return true;
}
