//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/StringUtil.h>
#include <IceUtil/FileUtil.h>
#include <IceUtil/Config.h>
#include <IcePatch2/ClientUtil.h>
#include <IcePatch2Lib/Util.h>
#include <list>
#include <iterator>
#include <optional>

using namespace std;
using namespace Ice;
using namespace IceUtil;
using namespace IcePatch2;
using namespace IcePatch2Internal;

IcePatch2::Patcher::~Patcher()
{
    // Out of line to avoid weak vtable
}

IcePatch2::PatcherFeedback::~PatcherFeedback()
{
    // Out of line to avoid weak vtable
}

namespace
{
    class Decompressor
    {
    public:
        Decompressor(const string&);
        virtual ~Decompressor();

        void destroy();
        void add(const LargeFileInfo&);
        void exception() const;
        void log(FILE* fp);
        virtual void run();

    private:
        const string _dataDir;

        string _exception;
        list<LargeFileInfo> _files;
        LargeFileInfoSeq _filesDone;
        bool _destroy;
        mutable std::mutex _mutex;
        std::condition_variable _conditionVariable;
    };
    using DecompressorPtr = std::shared_ptr<Decompressor>;

    class PatcherI final : public Patcher
    {
    public:
        PatcherI(const FileServerPrx&, const PatcherFeedbackPtr&, const std::string&, bool, int32_t, int32_t);
        ~PatcherI();

        bool prepare() final;
        bool patch(const std::string&) final;
        void finish() final;

    private:
        bool removeFiles(const LargeFileInfoSeq&);
        bool updateFiles(const LargeFileInfoSeq&);
        bool updateFilesInternal(const LargeFileInfoSeq&, const DecompressorPtr&);
        bool updateFlags(const LargeFileInfoSeq&);

        const PatcherFeedbackPtr _feedback;
        const std::string _dataDir;
        const bool _thorough;
        const int32_t _chunkSize;
        const int32_t _remove;
        const FileServerPrx _serverCompress;
        const FileServerPrx _serverNoCompress;

        LargeFileInfoSeq _localFiles;
        LargeFileInfoSeq _updateFiles;
        LargeFileInfoSeq _updateFlags;
        LargeFileInfoSeq _removeFiles;

        FILE* _log;
    };

    Decompressor::Decompressor(const string& dataDir) : _dataDir(dataDir), _destroy(false) {}

    Decompressor::~Decompressor() { assert(_destroy); }

    void Decompressor::destroy()
    {
        lock_guard lock(_mutex);
        _destroy = true;
        _conditionVariable.notify_one();
    }

    void Decompressor::add(const LargeFileInfo& info)
    {
        lock_guard lock(_mutex);
        if (!_exception.empty())
        {
            throw runtime_error(_exception);
        }
        _files.push_back(info);
        _conditionVariable.notify_one();
    }

    void Decompressor::exception() const
    {
        lock_guard lock(_mutex);
        if (!_exception.empty())
        {
            throw runtime_error(_exception);
        }
    }

    void Decompressor::log(FILE* fp)
    {
        lock_guard lock(_mutex);

        for (LargeFileInfoSeq::const_iterator p = _filesDone.begin(); p != _filesDone.end(); ++p)
        {
            if (fputc('+', fp) == EOF || !writeFileInfo(fp, *p))
            {
                throw runtime_error("error writing log file:\n" + IceUtilInternal::lastErrorToString());
            }
        }

        _filesDone.clear();
    }

    void Decompressor::run()
    {
        LargeFileInfo info;

        while (true)
        {
            {
                unique_lock lock(_mutex);

                if (!info.path.empty())
                {
                    _filesDone.push_back(info);
                }

                _conditionVariable.wait(lock, [this] { return _destroy || !_files.empty(); });
                if (_files.empty())
                {
                    return;
                }
                else
                {
                    info = _files.front();
                    _files.pop_front();
                }
            }

            try
            {
                decompressFile(_dataDir + '/' + info.path);
                setFileFlags(_dataDir + '/' + info.path, info);
                remove(_dataDir + '/' + info.path + ".bz2");
            }
            catch (const std::exception& ex)
            {
                lock_guard lock(_mutex);
                _destroy = true;
                _exception = ex.what();
                return;
            }
        }
    }

    PatcherI::PatcherI(
        const FileServerPrx& server,
        const PatcherFeedbackPtr& feedback,
        const string& dataDir,
        bool thorough,
        int32_t chunkSize,
        int32_t remove)
        : _feedback(feedback),
          _dataDir(dataDir),
          _thorough(thorough),
          _chunkSize(chunkSize),
          _remove(remove),
          _serverCompress(server->ice_compress(true)),
          _serverNoCompress(server->ice_compress(false))
    {
        if (_dataDir.empty())
        {
            throw runtime_error("no data directory specified");
        }

        Ice::CommunicatorPtr communicator = server->ice_getCommunicator();

        const_cast<string&>(_dataDir) = simplify(_dataDir);

        // Make sure that _chunkSize doesn't exceed MessageSizeMax, otherwise it won't work at all.
        int sizeMax = communicator->getProperties()->getPropertyAsIntWithDefault("Ice.MessageSizeMax", 1024);
        if (_chunkSize < 1)
        {
            const_cast<int32_t&>(_chunkSize) = 1;
        }
        else if (_chunkSize > sizeMax)
        {
            const_cast<int32_t&>(_chunkSize) = sizeMax;
        }

        if (_chunkSize == sizeMax)
        {
            const_cast<int32_t&>(_chunkSize) = _chunkSize * 1024 - 512; // Leave some headroom for protocol header.
        }
        else
        {
            const_cast<int32_t&>(_chunkSize) *= 1024;
        }

        if (!IceUtilInternal::isAbsolutePath(_dataDir))
        {
            string cwd;
            if (IceUtilInternal::getcwd(cwd) != 0)
            {
                throw runtime_error("cannot get the current directory:\n" + IceUtilInternal::lastErrorToString());
            }
            const_cast<string&>(_dataDir) = simplify(cwd + '/' + _dataDir);
        }
    }

    PatcherI::~PatcherI()
    {
        if (_log != 0)
        {
            fclose(_log);
            _log = 0;
        }
    }

    class GetFileInfoSeqAsyncCB final
    {
    public:
        GetFileInfoSeqAsyncCB() { _largeFileInfoSeqFuture = _largeFileInfoSeqPromise.get_future(); }

        void complete(LargeFileInfoSeq largeFileInfoSeq) { _largeFileInfoSeqPromise.set_value(largeFileInfoSeq); }

        void exception(std::exception_ptr exception) { _largeFileInfoSeqPromise.set_exception(exception); }

        LargeFileInfoSeq getLargeFileInfoSeq() { return _largeFileInfoSeqFuture.get(); }

    private:
        std::promise<LargeFileInfoSeq> _largeFileInfoSeqPromise;
        std::future<LargeFileInfoSeq> _largeFileInfoSeqFuture;
    };

    class PatcherGetFileInfoSeqCB final : public GetFileInfoSeqCB
    {
    public:
        PatcherGetFileInfoSeqCB(const PatcherFeedbackPtr& feedback) : _feedback(feedback) {}

        bool remove(const string&) final { return true; }

        bool checksum(const string& path) final { return _feedback->checksumProgress(path); }

        bool compress(const string&) final
        {
            assert(false); // Nothing must get compressed when we are patching.
            return true;
        }

    private:
        const PatcherFeedbackPtr _feedback;
    };

    bool PatcherI::prepare()
    {
        _localFiles.clear();

        bool thorough = _thorough;

        if (!thorough)
        {
            try
            {
                loadFileInfoSeq(_dataDir, _localFiles);
            }
            catch (const exception& ex)
            {
                thorough = _feedback->noFileSummary(ex.what());
                if (!thorough)
                {
                    return false;
                }
            }
        }

        if (thorough)
        {
            if (!_feedback->checksumStart())
            {
                return false;
            }

            PatcherGetFileInfoSeqCB cb(_feedback);
            if (!getFileInfoSeq(_dataDir, 0, &cb, _localFiles))
            {
                return false;
            }

            if (!_feedback->checksumEnd())
            {
                return false;
            }

            saveFileInfoSeq(_dataDir, _localFiles);
        }

        FileTree0 tree0;
        getFileTree0(_localFiles, tree0);

        if (tree0.checksum != _serverCompress->getChecksum())
        {
            if (!_feedback->fileListStart())
            {
                return false;
            }

            ByteSeqSeq checksumSeq = _serverCompress->getChecksumSeq();
            if (checksumSeq.size() != 256)
            {
                throw runtime_error("server returned illegal value");
            }

            std::shared_ptr<GetFileInfoSeqAsyncCB> curCB;
            std::shared_ptr<GetFileInfoSeqAsyncCB> nxtCB;
            for (size_t node0 = 0; node0 < 256; ++node0)
            {
                if (tree0.nodes[node0].checksum != checksumSeq[node0])
                {
                    if (!curCB)
                    {
                        curCB = std::make_shared<GetFileInfoSeqAsyncCB>();
                        _serverCompress->getLargeFileInfoSeqAsync(
                            static_cast<int32_t>(node0),
                            [curCB](LargeFileInfoSeq fileInfoSeq) { curCB->complete(fileInfoSeq); },
                            [curCB](exception_ptr exception) { curCB->exception(exception); });
                    }
                    else
                    {
                        assert(nxtCB);
                        swap(nxtCB, curCB);
                    }

                    size_t node0Nxt = node0;
                    do
                    {
                        ++node0Nxt;
                    } while (node0Nxt < 256 && tree0.nodes[node0Nxt].checksum == checksumSeq[node0Nxt]);

                    if (node0Nxt < 256)
                    {
                        nxtCB = std::make_shared<GetFileInfoSeqAsyncCB>();
                        _serverCompress->getLargeFileInfoSeqAsync(
                            static_cast<int32_t>(node0Nxt),
                            [nxtCB](LargeFileInfoSeq fileInfoSeq) { nxtCB->complete(fileInfoSeq); },
                            [nxtCB](exception_ptr exception) { nxtCB->exception(exception); });
                    }

                    LargeFileInfoSeq files = curCB->getLargeFileInfoSeq();
                    sort(files.begin(), files.end(), FileInfoLess());
                    files.erase(unique(files.begin(), files.end(), FileInfoEqual()), files.end());

                    //
                    // Compute the set of files which were removed.
                    //
                    set_difference(
                        tree0.nodes[node0].files.begin(),
                        tree0.nodes[node0].files.end(),
                        files.begin(),
                        files.end(),
                        back_inserter(_removeFiles),
                        FileInfoWithoutFlagsLess()); // NOTE: We ignore the flags here.

                    //
                    // Compute the set of files which were updated (either the file contents, flags or both).
                    //
                    LargeFileInfoSeq updatedFiles;
                    updatedFiles.reserve(files.size());

                    set_difference(
                        files.begin(),
                        files.end(),
                        tree0.nodes[node0].files.begin(),
                        tree0.nodes[node0].files.end(),
                        back_inserter(updatedFiles),
                        FileInfoLess());

                    //
                    // Compute the set of files whose contents was updated.
                    //
                    LargeFileInfoSeq contentsUpdatedFiles;
                    contentsUpdatedFiles.reserve(files.size());

                    set_difference(
                        files.begin(),
                        files.end(),
                        tree0.nodes[node0].files.begin(),
                        tree0.nodes[node0].files.end(),
                        back_inserter(contentsUpdatedFiles),
                        FileInfoWithoutFlagsLess()); // NOTE: We ignore the flags here.
                    copy(contentsUpdatedFiles.begin(), contentsUpdatedFiles.end(), back_inserter(_updateFiles));

                    //
                    // Compute the set of files whose flags were updated.
                    //
                    set_difference(
                        updatedFiles.begin(),
                        updatedFiles.end(),
                        contentsUpdatedFiles.begin(),
                        contentsUpdatedFiles.end(),
                        back_inserter(_updateFlags),
                        FileInfoLess());
                }

                if (!_feedback->fileListProgress(static_cast<int32_t>(node0 + 1) * 100 / 256))
                {
                    return false;
                }
            }

            if (!_feedback->fileListEnd())
            {
                return false;
            }
        }

        sort(_removeFiles.begin(), _removeFiles.end(), FileInfoLess());
        sort(_updateFiles.begin(), _updateFiles.end(), FileInfoLess());
        sort(_updateFlags.begin(), _updateFlags.end(), FileInfoLess());

        string pathLog = simplify(_dataDir + '/' + logFile);
        _log = IceUtilInternal::fopen(pathLog, "w");
        if (!_log)
        {
            throw runtime_error("cannot open `" + pathLog + "' for writing:\n" + IceUtilInternal::lastErrorToString());
        }

        return true;
    }

    bool PatcherI::patch(const string& d)
    {
        string dir = simplify(d);

        if (dir.empty() || dir == ".")
        {
            if (!_removeFiles.empty())
            {
                if (!removeFiles(_removeFiles))
                {
                    return false;
                }
            }

            if (!_updateFiles.empty())
            {
                if (!updateFiles(_updateFiles))
                {
                    return false;
                }
            }

            if (!_updateFlags.empty())
            {
                if (!updateFlags(_updateFlags))
                {
                    return false;
                }
            }

            return true;
        }
        else
        {
            string dirWithSlash = simplify(dir + '/');

            LargeFileInfoSeq remove;
            for (LargeFileInfoSeq::const_iterator p = _removeFiles.begin(); p != _removeFiles.end(); ++p)
            {
                if (p->path == dir)
                {
                    remove.push_back(*p);
                }
                else if (p->path.compare(0, dirWithSlash.size(), dirWithSlash) == 0)
                {
                    remove.push_back(*p);
                }
            }

            LargeFileInfoSeq update;
            for (LargeFileInfoSeq::const_iterator p = _updateFiles.begin(); p != _updateFiles.end(); ++p)
            {
                if (p->path == dir)
                {
                    update.push_back(*p);
                }
                else if (p->path.compare(0, dirWithSlash.size(), dirWithSlash) == 0)
                {
                    update.push_back(*p);
                }
            }

            LargeFileInfoSeq updateFlag;
            for (LargeFileInfoSeq::const_iterator p = _updateFlags.begin(); p != _updateFlags.end(); ++p)
            {
                if (p->path == dir)
                {
                    updateFlag.push_back(*p);
                }
                else if (p->path.compare(0, dirWithSlash.size(), dirWithSlash) == 0)
                {
                    updateFlag.push_back(*p);
                }
            }

            if (!remove.empty())
            {
                if (!removeFiles(remove))
                {
                    return false;
                }
            }

            if (!update.empty())
            {
                if (!updateFiles(update))
                {
                    return false;
                }
            }

            if (!updateFlag.empty())
            {
                if (!updateFlags(updateFlag))
                {
                    return false;
                }
            }

            return true;
        }
    }

    void PatcherI::finish()
    {
        if (_log != 0)
        {
            fclose(_log);
            _log = 0;
        }

        saveFileInfoSeq(_dataDir, _localFiles);
    }

    bool PatcherI::removeFiles(const LargeFileInfoSeq& files)
    {
        if (_remove < 1)
        {
            return true;
        }

        for (LargeFileInfoSeq::const_reverse_iterator p = files.rbegin(); p != files.rend(); ++p)
        {
            try
            {
                remove(_dataDir + '/' + p->path);
                if (fputc('-', _log) == EOF || !writeFileInfo(_log, *p))
                {
                    throw runtime_error("error writing log file:\n" + IceUtilInternal::lastErrorToString());
                }
            }
            catch (...)
            {
                if (_remove < 2) // We ignore errors if IcePatch2Client.Remove >= 2.
                {
                    throw;
                }
            }
        }

        LargeFileInfoSeq newLocalFiles;
        newLocalFiles.reserve(_localFiles.size());

        set_difference(
            _localFiles.begin(),
            _localFiles.end(),
            files.begin(),
            files.end(),
            back_inserter(newLocalFiles),
            FileInfoLess());

        _localFiles.swap(newLocalFiles);

        LargeFileInfoSeq newRemoveFiles;

        set_difference(
            _removeFiles.begin(),
            _removeFiles.end(),
            files.begin(),
            files.end(),
            back_inserter(newRemoveFiles),
            FileInfoLess());

        _removeFiles.swap(newRemoveFiles);

        return true;
    }

    bool PatcherI::updateFiles(const LargeFileInfoSeq& files)
    {
        auto decompressor = make_shared<Decompressor>(_dataDir);
        bool result;

        thread decompressorThread = std::thread([decompressor] { decompressor->run(); });
        try
        {
            result = updateFilesInternal(files, decompressor);
        }
        catch (...)
        {
            decompressor->destroy();
            if (decompressorThread.joinable())
            {
                decompressorThread.join();
            }
            decompressor->log(_log);
            throw;
        }

        decompressor->destroy();
        if (decompressorThread.joinable())
        {
            decompressorThread.join();
        }
        decompressor->log(_log);
        decompressor->exception();

        return result;
    }

    class GetFileCompressedCB
    {
    public:
        GetFileCompressedCB() { _bytesFuture = _bytesPromise.get_future(); }

        void complete(ByteSeq bytes) { _bytesPromise.set_value(bytes); }

        void exception(std::exception_ptr exception) { _bytesPromise.set_exception(exception); }

        ByteSeq getFileCompressed() { return _bytesFuture.get(); }

    private:
        std::promise<ByteSeq> _bytesPromise;
        std::future<ByteSeq> _bytesFuture;
    };

    void getFileCompressed(
        FileServerPrx serverNoCompress,
        std::string path,
        int64_t pos,
        int chunkSize,
        std::shared_ptr<GetFileCompressedCB> cb)
    {
        serverNoCompress->getLargeFileCompressedAsync(
            path,
            pos,
            chunkSize,
            [cb](std::pair<const byte*, const byte*> result) { cb->complete(ByteSeq(result.first, result.second)); },
            [cb](exception_ptr exception) { cb->exception(exception); });
    }

    bool PatcherI::updateFilesInternal(const LargeFileInfoSeq& files, const DecompressorPtr& decompressor)
    {
        int64_t total = 0;
        int64_t updated = 0;

        for (LargeFileInfoSeq::const_iterator p = files.begin(); p != files.end(); ++p)
        {
            if (p->size > 0) // Regular, non-empty file?
            {
                total += p->size;
            }
        }

        shared_ptr<GetFileCompressedCB> curCB;
        shared_ptr<GetFileCompressedCB> nxtCB;

        for (LargeFileInfoSeq::const_iterator p = files.begin(); p != files.end(); ++p)
        {
            if (p->size < 0) // Directory?
            {
                createDirectoryRecursive(_dataDir + '/' + p->path);
                if (fputc('+', _log) == EOF || !writeFileInfo(_log, *p))
                {
                    throw runtime_error("error writing log file:\n" + IceUtilInternal::lastErrorToString());
                }
            }
            else // Regular file.
            {
                if (!_feedback->patchStart(p->path, p->size, updated, total))
                {
                    return false;
                }

                if (p->size == 0)
                {
                    string path = simplify(_dataDir + '/' + p->path);
                    FILE* fp = IceUtilInternal::fopen(path, "wb");
                    if (fp == 0)
                    {
                        throw runtime_error(
                            "cannot open `" + path + "' for writing:\n" + IceUtilInternal::lastErrorToString());
                    }
                    fclose(fp);
                }
                else
                {
                    string pathBZ2 = simplify(_dataDir + '/' + p->path + ".bz2");

                    string dir = getDirname(pathBZ2);
                    if (!dir.empty())
                    {
                        createDirectoryRecursive(dir);
                    }

                    try
                    {
                        removeRecursive(pathBZ2);
                    }
                    catch (...)
                    {
                    }

                    FILE* fileBZ2 = IceUtilInternal::fopen(pathBZ2, "wb");
                    if (fileBZ2 == 0)
                    {
                        throw runtime_error(
                            "cannot open `" + pathBZ2 + "' for writing:\n" + IceUtilInternal::lastErrorToString());
                    }

                    try
                    {
                        int64_t pos = 0;

                        while (pos < p->size)
                        {
                            if (!curCB)
                            {
                                assert(!nxtCB);
                                curCB = std::make_shared<GetFileCompressedCB>();
                                getFileCompressed(_serverNoCompress, p->path, pos, _chunkSize, curCB);
                            }
                            else
                            {
                                assert(nxtCB);
                                swap(nxtCB, curCB);
                            }

                            if (pos + _chunkSize < p->size)
                            {
                                nxtCB = std::make_shared<GetFileCompressedCB>();
                                getFileCompressed(_serverNoCompress, p->path, pos + _chunkSize, _chunkSize, nxtCB);
                            }
                            else
                            {
                                LargeFileInfoSeq::const_iterator q = p + 1;

                                while (q != files.end() && q->size <= 0)
                                {
                                    ++q;
                                }

                                if (q != files.end())
                                {
                                    nxtCB = std::make_shared<GetFileCompressedCB>();
                                    getFileCompressed(_serverNoCompress, q->path, 0, _chunkSize, nxtCB);
                                }
                            }

                            ByteSeq bytes;
                            try
                            {
                                bytes = curCB->getFileCompressed();
                            }
                            catch (const FileAccessException& ex)
                            {
                                throw runtime_error("error from IcePatch2 server for `" + p->path + "': " + ex.reason);
                            }

                            if (bytes.empty())
                            {
                                throw runtime_error("size mismatch for `" + p->path + "'");
                            }

                            if (fwrite(reinterpret_cast<char*>(&bytes[0]), bytes.size(), 1, fileBZ2) != 1)
                            {
                                throw runtime_error(
                                    ": cannot write `" + pathBZ2 + "':\n" + IceUtilInternal::lastErrorToString());
                            }

                            // 'bytes' is always returned with size '_chunkSize'. When a file is smaller than
                            // '_chunkSize' or we are reading the last chunk of a file, 'bytes' will be larger than
                            // necessary. In this case we calculate the current position and updated size based on the
                            // known file size.
                            size_t size = (static_cast<size_t>(pos) + bytes.size()) > static_cast<size_t>(p->size)
                                              ? static_cast<size_t>(p->size - pos)
                                              : bytes.size();

                            pos += size;
                            updated += size;

                            if (!_feedback->patchProgress(pos, p->size, updated, total))
                            {
                                fclose(fileBZ2);
                                return false;
                            }
                        }
                    }
                    catch (...)
                    {
                        fclose(fileBZ2);
                        throw;
                    }

                    fclose(fileBZ2);

                    decompressor->log(_log);
                    decompressor->add(*p);
                }

                if (!_feedback->patchEnd())
                {
                    return false;
                }
            }
        }

        LargeFileInfoSeq newLocalFiles;
        newLocalFiles.reserve(_localFiles.size());

        set_union(
            _localFiles.begin(),
            _localFiles.end(),
            files.begin(),
            files.end(),
            back_inserter(newLocalFiles),
            FileInfoLess());

        _localFiles.swap(newLocalFiles);

        LargeFileInfoSeq newUpdateFiles;

        set_difference(
            _updateFiles.begin(),
            _updateFiles.end(),
            files.begin(),
            files.end(),
            back_inserter(newUpdateFiles),
            FileInfoLess());

        _updateFiles.swap(newUpdateFiles);

        return true;
    }

    bool PatcherI::updateFlags(const LargeFileInfoSeq& files)
    {
        for (LargeFileInfoSeq::const_iterator p = files.begin(); p != files.end(); ++p)
        {
            if (p->size >= 0) // Regular file?
            {
                setFileFlags(_dataDir + '/' + p->path, *p);
            }
        }

        //
        // Remove the old files whose flags were updated from the set of
        // local files.
        //
        LargeFileInfoSeq localFiles;
        localFiles.reserve(_localFiles.size());
        set_difference(
            _localFiles.begin(),
            _localFiles.end(),
            files.begin(),
            files.end(),
            back_inserter(localFiles),
            FileInfoWithoutFlagsLess()); // NOTE: We ignore the flags.

        //
        // Add the new files to the set of local file.
        //
        _localFiles.clear();
        set_union(
            localFiles.begin(),
            localFiles.end(),
            files.begin(),
            files.end(),
            back_inserter(_localFiles),
            FileInfoLess());

        LargeFileInfoSeq newUpdateFlags;

        set_difference(
            _updateFlags.begin(),
            _updateFlags.end(),
            files.begin(),
            files.end(),
            back_inserter(newUpdateFlags),
            FileInfoLess());

        _updateFlags.swap(newUpdateFlags);

        return true;
    }
}

//
// Create a patcher with the given parameters. These parameters
// are equivalent to the configuration properties described above.
//
PatcherPtr
PatcherFactory::create(
    const FileServerPrx& server,
    const PatcherFeedbackPtr& feedback,
    const string& dataDir,
    bool thorough,
    int32_t chunkSize,
    int32_t remove)
{
    return make_shared<PatcherI>(server, feedback, dataDir, thorough, chunkSize, remove);
}
