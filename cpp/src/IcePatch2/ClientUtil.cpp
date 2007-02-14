// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Unicode.h>
#include <IcePatch2/ClientUtil.h>
#include <IcePatch2/Util.h>
#include <IcePatch2/FileServerI.h>
#include <list>
#include <OS.h>

#ifdef __BCPLUSPLUS__
#  include <iterator>
#endif

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
                throw "error writing log file:\n" + lastError();
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

IcePatch2::Patcher::Patcher(const CommunicatorPtr& communicator, const PatcherFeedbackPtr& feedback) :
    _feedback(feedback),
    _dataDir(simplify(communicator->getProperties()->getPropertyWithDefault("IcePatch2.Directory", "."))),
    _thorough(communicator->getProperties()->getPropertyAsInt("IcePatch2.Thorough") > 0),
    _chunkSize(communicator->getProperties()->getPropertyAsIntWithDefault("IcePatch2.ChunkSize", 100)),
    _remove(communicator->getProperties()->getPropertyAsIntWithDefault("IcePatch2.Remove", 1)),
    _log(0)
{
    PropertiesPtr properties = communicator->getProperties();

    const char* endpointsProperty = "IcePatch2.Endpoints";
    string endpoints = properties->getProperty(endpointsProperty);
    if(endpoints.empty())
    {
        throw string("property `") + endpointsProperty + "' is not set";
    }

    Identity id;
    id.category = properties->getPropertyWithDefault("IcePatch2.InstanceName", "IcePatch2");
    id.name = "server";
    
    ObjectPrx serverBase = communicator->stringToProxy("\"" + communicator->identityToString(id) + "\" :" + endpoints);
    FileServerPrx server = FileServerPrx::checkedCast(serverBase);
    if(!server)
    {
        throw "proxy `" + communicator->identityToString(id) + ':' + endpoints + "' is not a file server.";
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
    _dataDir(simplify(dataDir)),
    _thorough(thorough),
    _chunkSize(chunkSize),
    _remove(remove)
{
    init(server);
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

class AMIGetFileInfoSeq : public AMI_FileServer_getFileInfoSeq, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    AMIGetFileInfoSeq() :
        _done(false)
    {
    }

    FileInfoSeq
    getFileInfoSeq()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        while(!_done)
        {
            wait();
        }

        _done = false;

        if(_exception.get())
        {
            auto_ptr<Exception> ex = _exception;
            _fileInfoSeq.clear();
            ex->ice_throw();
        }

        FileInfoSeq fileInfoSeq;
        fileInfoSeq.swap(_fileInfoSeq);
        return fileInfoSeq;
    }

    virtual void
    ice_response(const FileInfoSeq& fileInfoSeq)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        _fileInfoSeq = fileInfoSeq;
        _done = true;
        notify();
    }

    virtual void
    ice_exception(const Exception& ex)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        _exception.reset(ex.ice_clone());
        _done = true;
        notify();
    }

private:

    bool _done;
    FileInfoSeq _fileInfoSeq;
    auto_ptr<Exception> _exception;
};

typedef IceUtil::Handle<AMIGetFileInfoSeq> AMIGetFileInfoSeqPtr;

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
        
        AMIGetFileInfoSeqPtr curCB;
        AMIGetFileInfoSeqPtr nxtCB;

        for(int node0 = 0; node0 < 256; ++node0)
        {
            if(tree0.nodes[node0].checksum != checksumSeq[node0])
            {
                if(!curCB)
                {
                    assert(!nxtCB);
                    curCB = new AMIGetFileInfoSeq;
                    nxtCB = new AMIGetFileInfoSeq;
                    _serverCompress->getFileInfoSeq_async(curCB, node0);
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
                    _serverCompress->getFileInfoSeq_async(nxtCB, node0Nxt);
                }

                FileInfoSeq files = curCB->getFileInfoSeq();
                
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
    _log = OS::fopen(pathLog, "w");
    if(!_log)
    {
        throw "cannot open `" + pathLog + "' for writing:\n" + lastError();
    }

    return true;
}

bool
IcePatch2::Patcher::patch(const string& d)
{
    string dir = simplify(d);

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

        FileInfoSeq updateFlag;
        for(p = _updateFlags.begin(); p != _updateFlags.end(); ++p)
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

    //
    // Make sure that _chunkSize doesn't exceed MessageSizeMax, otherwise
    // it won't work at all.
    //
    int sizeMax = 
        server->ice_getCommunicator()->getProperties()->getPropertyAsIntWithDefault("Ice.MessageSizeMax", 1024);
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

    if(!isAbsolute(_dataDir))
    {
        string cwd;
        if(OS::getcwd(cwd) != 0)
        {
            throw "cannot get the current directory:\n" + lastError();
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
                throw "error writing log file:\n" + lastError();
            }
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

class AMIGetFileCompressed : public AMI_FileServer_getFileCompressed, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    AMIGetFileCompressed() :
        _done(false)
    {
    }

    ByteSeq
    getFileCompressed()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        while(!_done)
        {
            wait();
        }

        _done = false;

        if(_exception.get())
        {
            auto_ptr<Exception> ex = _exception;
            _bytes.clear();
            ex->ice_throw();
        }

        ByteSeq bytes;
        bytes.swap(_bytes);
        return bytes;
    }

    virtual void
    ice_response(const pair<const Ice::Byte*, const Ice::Byte*>& bytes)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        ByteSeq(bytes.first, bytes.second).swap(_bytes);
        _done = true;
        notify();
    }

    virtual void
    ice_exception(const Exception& ex)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        _exception.reset(ex.ice_clone());
        _done = true;
        notify();
    }

private:

    bool _done;
    ByteSeq _bytes;
    auto_ptr<Exception> _exception;
};

typedef IceUtil::Handle<AMIGetFileCompressed> AMIGetFileCompressedPtr;

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
    
    AMIGetFileCompressedPtr curCB;
    AMIGetFileCompressedPtr nxtCB;

    for(p = files.begin(); p != files.end(); ++p)
    {
        if(p->size < 0) // Directory?
        {
            createDirectoryRecursive(_dataDir + '/' + p->path);
            if(fputc('+', _log) == EOF || !writeFileInfo(_log, *p))
            {
                throw "error writing log file:\n" + lastError();
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
                FILE* fp = OS::fopen(path, "wb");
                if(fp == 0)
                {
                    throw "cannot open `" + path +"' for writing:\n" + lastError();
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
                
                FILE* fileBZ2 = OS::fopen(pathBZ2, "wb");
                if(fileBZ2 == 0)
                {
                    throw "cannot open `" + pathBZ2 + "' for writing:\n" + lastError();
                }

                try
                {
                    Int pos = 0;

                    while(pos < p->size)
                    {
                        if(!curCB)
                        {
                            assert(!nxtCB);
                            curCB = new AMIGetFileCompressed;
                            nxtCB = new AMIGetFileCompressed;
                            _serverNoCompress->getFileCompressed_async(curCB, p->path, pos, _chunkSize);
                        }
                        else
                        {
                            assert(nxtCB);
                            swap(nxtCB, curCB);
                        }

                        if(pos + _chunkSize < p->size)
                        {
                            _serverNoCompress->getFileCompressed_async(nxtCB, p->path, pos + _chunkSize, _chunkSize);
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
                                _serverNoCompress->getFileCompressed_async(nxtCB, q->path, 0, _chunkSize);
                            }
                        }

                        ByteSeq bytes;

                        try
                        {
                            bytes = curCB->getFileCompressed();
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
                            throw ": cannot write `" + pathBZ2 + "':\n" + lastError();
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

