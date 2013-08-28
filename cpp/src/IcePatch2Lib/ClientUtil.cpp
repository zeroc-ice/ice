// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Unicode.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/FileUtil.h>
#include <Ice/StringConverter.h>
#define ICE_PATCH2_API_EXPORTS
#include <IcePatch2/ClientUtil.h>
#include <IcePatch2/Util.h>
#include <list>
#include <iterator>

using namespace std;
using namespace Ice;
using namespace IcePatch2;

namespace IcePatch2
{

class Decompressor : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
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
    add(const FileInfo& info)
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
    log(FILE* fp)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        for(FileInfoSeq::const_iterator p = _filesDone.begin(); p != _filesDone.end(); ++p)
        {
            if(fputc('+', fp) == EOF || !writeFileInfo(fp, *p))
            {
                throw "error writing log file:\n" + IceUtilInternal::lastErrorToString();
            }
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
                setFileFlags(_dataDir + '/' + info.path, info);
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

namespace
{

string
getDataDir(const CommunicatorPtr& communicator, const string& defaultValue)
{
    const string property = "IcePatch2Client.Directory";
    const string deprecatedProperty = "IcePatch2.Directory";

    if(communicator->getProperties()->getProperty(property).empty() &&
       communicator->getProperties()->getProperty(deprecatedProperty).empty())
    {
        return defaultValue;
    }

    string value = communicator->getProperties()->getProperty(property);
    if(value.empty())
    {
        value = communicator->getProperties()->getProperty(deprecatedProperty);
        ostringstream os;
        os << "The property " << deprecatedProperty << " is deprecated, use " << property << " instead.";
        communicator->getLogger()->warning(os.str());
    }
    assert(!value.empty());
    return value;
}

int
getThorough(const CommunicatorPtr& communicator, int defaultValue)
{
    const string property = "IcePatch2Client.Thorough";
    const string deprecatedProperty = "IcePatch2.Thorough";

    if(communicator->getProperties()->getProperty(property).empty() &&
       communicator->getProperties()->getProperty(deprecatedProperty).empty())
    {
        return defaultValue;
    }

    if(!communicator->getProperties()->getProperty(property).empty())
    {
        return communicator->getProperties()->getPropertyAsInt(property);
    }
    else
    {
        ostringstream os;
        os << "The property " << deprecatedProperty << " is deprecated, use " << property << " instead.";
        communicator->getLogger()->warning(os.str());
        return communicator->getProperties()->getPropertyAsInt(deprecatedProperty);
    }
}

int
getChunkSize(const CommunicatorPtr& communicator, int defaultValue)
{
    const string property = "IcePatch2Client.ChunkSize";
    const string deprecatedProperty = "IcePatch2.ChunkSize";

    if(communicator->getProperties()->getProperty(property).empty() &&
       communicator->getProperties()->getProperty(deprecatedProperty).empty())
    {
        return defaultValue;
    }

    if(!communicator->getProperties()->getProperty(property).empty())
    {
        return communicator->getProperties()->getPropertyAsInt(property);
    }
    else
    {
        ostringstream os;
        os << "The property " << deprecatedProperty << " is deprecated, use " << property << " instead.";
        communicator->getLogger()->warning(os.str());
        return communicator->getProperties()->getPropertyAsInt(deprecatedProperty);
    }
}

int
getRemove(const CommunicatorPtr& communicator, int defaultValue)
{
    const string property = "IcePatch2Client.Remove";
    const string deprecatedProperty = "IcePatch2.Remove";

    if(communicator->getProperties()->getProperty(property).empty() &&
       communicator->getProperties()->getProperty(deprecatedProperty).empty())
    {
        return defaultValue;
    }

    if(!communicator->getProperties()->getProperty(property).empty())
    {
        return communicator->getProperties()->getPropertyAsInt(property);
    }
    else
    {
        ostringstream os;
        os << "The property " << deprecatedProperty << " is deprecated, use " << property << " instead.";
        communicator->getLogger()->warning(os.str());
        return communicator->getProperties()->getPropertyAsInt(deprecatedProperty);
    }
}

}


IcePatch2::Patcher::Patcher(const CommunicatorPtr& communicator, const PatcherFeedbackPtr& feedback) :
    _feedback(feedback),
    _dataDir(getDataDir(communicator, ".")),
    _thorough(getThorough(communicator, 0) > 0),
    _chunkSize(getChunkSize(communicator, 100)),
    _remove(getRemove(communicator, 1)),
    _log(0)
{
    const PropertiesPtr properties = communicator->getProperties();
    const char* clientProxyProperty = "IcePatch2Client.Proxy";
    std::string clientProxy = properties->getProperty(clientProxyProperty);
    if(clientProxy.empty())
    {
        const char* endpointsProperty = "IcePatch2.Endpoints";
        string endpoints = properties->getProperty(endpointsProperty);
        if(endpoints.empty())
        {
            ostringstream os;
            os << "No proxy to IcePatch2 server. Please set `" << clientProxyProperty 
               << "' or `" << endpointsProperty << "'.";
            throw os.str();
        }
        ostringstream os;
        os << "The property " << endpointsProperty << " is deprecated, use " << clientProxyProperty << " instead.";
        communicator->getLogger()->warning(os.str());
        Identity id;
        id.category = properties->getPropertyWithDefault("IcePatch2.InstanceName", "IcePatch2");
        id.name = "server";
        
        clientProxy = "\"" + communicator->identityToString(id) + "\" :" + endpoints;
    }
    ObjectPrx serverBase = communicator->stringToProxy(clientProxy);
    
    FileServerPrx server = FileServerPrx::checkedCast(serverBase);
    if(!server)
    {
        throw "proxy `" + clientProxy + "' is not a file server.";
    }

    init(server);
}

IcePatch2::Patcher::Patcher(const FileServerPrx& server,
                            const PatcherFeedbackPtr& feedback,
                            const string& dataDir,
                            bool thorough,
                            Ice::Int chunkSize,
                            Ice::Int remove) :
    _feedback(feedback),
    _dataDir(dataDir),
    _thorough(thorough),
    _chunkSize(chunkSize),
    _remove(remove)
{
    init(server);
}

IcePatch2::Patcher::~Patcher()
{
    if(_log != 0)
    {
        fclose(_log);
        _log = 0;
    }
}

namespace
{

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

}

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
        
        ByteSeqSeq checksumSeq = _serverCompress->getChecksumSeq();
        if(checksumSeq.size() != 256)
        {
            throw string("server returned illegal value");
        }
        
        AsyncResultPtr curCB;
        AsyncResultPtr nxtCB;

        for(int node0 = 0; node0 < 256; ++node0)
        {
            if(tree0.nodes[node0].checksum != checksumSeq[node0])
            {
                if(!curCB)
                {
                    assert(!nxtCB);
                    curCB = _serverCompress->begin_getFileInfoSeq(node0);
                }
                else
                {
                    assert(nxtCB);
                    swap(nxtCB, curCB);
                }
                
                int node0Nxt = node0;
                
                do
                {
                    ++node0Nxt;
                }
                while(node0Nxt < 256 && tree0.nodes[node0Nxt].checksum == checksumSeq[node0Nxt]);

                if(node0Nxt < 256)
                {
                    nxtCB = _serverCompress->begin_getFileInfoSeq(node0Nxt);
                }

                FileInfoSeq files = _serverCompress->end_getFileInfoSeq(curCB);
                
                sort(files.begin(), files.end(), FileInfoLess());
                files.erase(unique(files.begin(), files.end(), FileInfoEqual()), files.end());

                //
                // Compute the set of files which were removed.
                //
                set_difference(tree0.nodes[node0].files.begin(),
                               tree0.nodes[node0].files.end(),
                               files.begin(),
                               files.end(),
                               back_inserter(_removeFiles),
                               FileInfoWithoutFlagsLess()); // NOTE: We ignore the flags here.

                //
                // Compute the set of files which were updated (either the file contents, flags or both).
                //
                FileInfoSeq updatedFiles;
                updatedFiles.reserve(files.size());
                                    
                set_difference(files.begin(),
                               files.end(),
                               tree0.nodes[node0].files.begin(),
                               tree0.nodes[node0].files.end(),
                               back_inserter(updatedFiles),
                               FileInfoLess());

                //
                // Compute the set of files whose contents was updated.
                //
                FileInfoSeq contentsUpdatedFiles;
                contentsUpdatedFiles.reserve(files.size());

                set_difference(files.begin(),
                               files.end(),
                               tree0.nodes[node0].files.begin(),
                               tree0.nodes[node0].files.end(),
                               back_inserter(contentsUpdatedFiles),
                               FileInfoWithoutFlagsLess()); // NOTE: We ignore the flags here.
                copy(contentsUpdatedFiles.begin(), contentsUpdatedFiles.end(), back_inserter(_updateFiles));

                //
                // Compute the set of files whose flags were updated.
                //
                set_difference(updatedFiles.begin(),
                               updatedFiles.end(),
                               contentsUpdatedFiles.begin(),
                               contentsUpdatedFiles.end(),
                               back_inserter(_updateFlags),
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
    sort(_updateFlags.begin(), _updateFlags.end(), FileInfoLess());
                
    string pathLog = simplify(_dataDir + '/' + logFile);
    _log = IceUtilInternal::fopen(pathLog, "w");
    if(!_log)
    {
        throw "cannot open `" + pathLog + "' for writing:\n" + IceUtilInternal::lastErrorToString();
    }

    return true;
}

bool
IcePatch2::Patcher::patch(const string& d)
{
    string dir = simplify(nativeToUTF8(_serverNoCompress->ice_getCommunicator(), d));

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

        if(!_updateFlags.empty())
        {
            if(!updateFlags(_updateFlags))
            {
                return false;
            }
        }
        
        return true;
    }
    else
    {
        string dirWithSlash = simplify(dir + '/');

        FileInfoSeq remove;
        for(FileInfoSeq::const_iterator p = _removeFiles.begin(); p != _removeFiles.end(); ++p)
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
        for(FileInfoSeq::const_iterator p = _updateFiles.begin(); p != _updateFiles.end(); ++p)
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

        FileInfoSeq updateFlag;
        for(FileInfoSeq::const_iterator p = _updateFlags.begin(); p != _updateFlags.end(); ++p)
        {
            if(p->path == dir)
            {
                updateFlag.push_back(*p);
            }
            else if(p->path.compare(0, dirWithSlash.size(), dirWithSlash) == 0)
            {
                updateFlag.push_back(*p);
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
        
        if(!updateFlag.empty())
        {
            if(!updateFlags(updateFlag))
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
    if(_log != 0)
    {
        fclose(_log);
        _log = 0;
    }

    saveFileInfoSeq(_dataDir, _localFiles);
}

void
IcePatch2::Patcher::init(const FileServerPrx& server)
{
    if(_dataDir.empty())
    {
        throw string("no data directory specified");
    }

    Ice::CommunicatorPtr communicator = server->ice_getCommunicator();

    //
    // Transform dataDir to a UTF8 string (it's either read from the properties or 
    // provided by the user application directly).
    //
    const_cast<string&>(_dataDir) = simplify(nativeToUTF8(communicator, _dataDir));

    //
    // Make sure that _chunkSize doesn't exceed MessageSizeMax, otherwise
    // it won't work at all.
    //
    int sizeMax = communicator->getProperties()->getPropertyAsIntWithDefault("Ice.MessageSizeMax", 1024);
    if(_chunkSize < 1)
    {
        const_cast<Int&>(_chunkSize) = 1;
    }
    else if(_chunkSize > sizeMax)
    {
        const_cast<Int&>(_chunkSize) = sizeMax;
    }
    if(_chunkSize == sizeMax)
    {
        const_cast<Int&>(_chunkSize) = _chunkSize * 1024 - 512; // Leave some headroom for protocol header.
    }
    else
    {
        const_cast<Int&>(_chunkSize) *= 1024;
    }

    if(!IceUtilInternal::isAbsolutePath(_dataDir))
    {
        string cwd;
        if(IceUtilInternal::getcwd(cwd) != 0)
        {
            throw "cannot get the current directory:\n" + IceUtilInternal::lastErrorToString();
        }
        const_cast<string&>(_dataDir) = simplify(cwd + '/' + _dataDir);
    }
        
    const_cast<FileServerPrx&>(_serverCompress) = FileServerPrx::uncheckedCast(server->ice_compress(true));
    const_cast<FileServerPrx&>(_serverNoCompress) = FileServerPrx::uncheckedCast(server->ice_compress(false));
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
            if(fputc('-', _log) == EOF || ! writeFileInfo(_log, *p))
            {
                throw "error writing log file:\n" + IceUtilInternal::lastErrorToString();
            }
        }
        catch(...)
        {
            if(_remove < 2) // We ignore errors if IcePatch2Client.Remove >= 2.
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
#if defined(__hppa)
    //
    // The thread stack size is only 64KB only HP-UX and that's not
    // enough for this thread.
    //
    decompressor->start(256 * 1024); // 256KB
#else 
    decompressor->start();
#endif
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
    Long total = 0;
    Long updated = 0;
    
    for(FileInfoSeq::const_iterator p = files.begin(); p != files.end(); ++p)
    {
        if(p->size > 0) // Regular, non-empty file?
        {
            total += p->size;
        }
    }
    
    AsyncResultPtr curCB;
    AsyncResultPtr nxtCB;

    for(FileInfoSeq::const_iterator p = files.begin(); p != files.end(); ++p)
    {
        if(p->size < 0) // Directory?
        {
            createDirectoryRecursive(_dataDir + '/' + p->path);
            if(fputc('+', _log) == EOF || !writeFileInfo(_log, *p))
            {
                throw "error writing log file:\n" + IceUtilInternal::lastErrorToString();
            }
        }
        else // Regular file.
        {
            if(!_feedback->patchStart(p->path, p->size, updated, total))
            {
                return false;
            }

            if(p->size == 0)
            {
                string path = simplify(_dataDir + '/' + p->path);
                FILE* fp = IceUtilInternal::fopen(path, "wb");
                if(fp == 0)
                {
                    throw "cannot open `" + path +"' for writing:\n" + IceUtilInternal::lastErrorToString();
                }
                fclose(fp);
            }
            else
            {
                string pathBZ2 = simplify(_dataDir + '/' + p->path + ".bz2");
            
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
                
                FILE* fileBZ2 = IceUtilInternal::fopen(pathBZ2, "wb");
                if(fileBZ2 == 0)
                {
                    throw "cannot open `" + pathBZ2 + "' for writing:\n" + IceUtilInternal::lastErrorToString();
                }

                try
                {
                    Int pos = 0;

                    while(pos < p->size)
                    {
                        if(!curCB)
                        {
                            assert(!nxtCB);
                            curCB = _serverNoCompress->begin_getFileCompressed(p->path, pos, _chunkSize);
                        }
                        else
                        {
                            assert(nxtCB);
                            swap(nxtCB, curCB);
                        }

                        if(pos + _chunkSize < p->size)
                        {
                            nxtCB = _serverNoCompress->begin_getFileCompressed(p->path, pos + _chunkSize, _chunkSize);
                        }
                        else
                        {
                            FileInfoSeq::const_iterator q = p + 1;

                            while(q != files.end() && q->size <= 0)
                            {
                                ++q;
                            }

                            if(q != files.end())
                            {
                                nxtCB = _serverNoCompress->begin_getFileCompressed(q->path, 0, _chunkSize);
                            }
                        }

                        ByteSeq bytes;

                        try
                        {
                            bytes = _serverNoCompress->end_getFileCompressed(curCB);
                        }
                        catch(const FileAccessException& ex)
                        {
                            throw "error from IcePatch2 server for `" + p->path + "': " + ex.reason;
                        }

                        if(bytes.empty())
                        {
                            throw "size mismatch for `" + p->path + "'";
                        }

                        if(fwrite(reinterpret_cast<char*>(&bytes[0]), bytes.size(), 1, fileBZ2) != 1)
                        {
                            throw ": cannot write `" + pathBZ2 + "':\n" + IceUtilInternal::lastErrorToString();
                        }

                        pos += static_cast<int>(bytes.size());
                        updated += bytes.size();

                        if(!_feedback->patchProgress(pos, p->size, updated, total))
                        {
                            fclose(fileBZ2);
                            return false;
                        }
                    }
                }
                catch(...)
                {
                    fclose(fileBZ2);
                    throw;
                }
                
                fclose(fileBZ2);
                
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

bool
IcePatch2::Patcher::updateFlags(const FileInfoSeq& files)
{
    for(FileInfoSeq::const_iterator p = files.begin(); p != files.end(); ++p)
    {
        if(p->size >= 0) // Regular file?
        {
            setFileFlags(_dataDir + '/' + p->path, *p);
        }
    }

    //
    // Remove the old files whose flags were updated from the set of
    // local files.
    // 
    FileInfoSeq localFiles;
    localFiles.reserve(_localFiles.size());
    set_difference(_localFiles.begin(),
                   _localFiles.end(),
                   files.begin(),
                   files.end(),
                   back_inserter(localFiles),
                   FileInfoWithoutFlagsLess()); // NOTE: We ignore the flags.

    //
    // Add the new files to the set of local file. 
    //
    _localFiles.clear();
    set_union(localFiles.begin(),
              localFiles.end(),
              files.begin(),
              files.end(),
              back_inserter(_localFiles),
              FileInfoLess());

    FileInfoSeq newUpdateFlags;

    set_difference(_updateFlags.begin(),
                   _updateFlags.end(),
                   files.begin(),
                   files.end(),
                   back_inserter(newUpdateFlags),
                   FileInfoLess());
        
    _updateFlags.swap(newUpdateFlags);

    return true;
}

