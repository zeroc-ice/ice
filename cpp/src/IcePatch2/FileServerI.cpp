// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/ScopedArray.h>
#include <IceUtil/Unicode.h>
#include <IcePatch2/FileServerI.h>
#include <OS.h>

#ifdef _WIN32
#   include <io.h>
#else
#   include <unistd.h>
#endif

using namespace std;
using namespace Ice;
using namespace IcePatch2;

IcePatch2::FileServerI::FileServerI(const std::string& dataDir, const FileInfoSeq& infoSeq) :
    _dataDir(dataDir), _tree0(FileTree0())
{
    FileTree0& tree0 = const_cast<FileTree0&>(_tree0);
    getFileTree0(infoSeq, tree0);
}

FileInfoSeq
IcePatch2::FileServerI::getFileInfoSeq(Int node0, const Current&) const
{
    if(node0 < 0 || node0 > 255)
    {
        throw PartitionOutOfRangeException();
    }

    return _tree0.nodes[node0].files;
}

ByteSeqSeq
IcePatch2::FileServerI::getChecksumSeq(const Current&) const
{
    ByteSeqSeq checksums(256);

    for(int node0 = 0; node0 < 256; ++node0)
    {
        checksums[node0] = _tree0.nodes[node0].checksum;
    }

    return checksums;
}

ByteSeq
IcePatch2::FileServerI::getChecksum(const Current&) const
{
    return _tree0.checksum;
}

void
IcePatch2::FileServerI::getFileCompressed_async(const AMD_FileServer_getFileCompressedPtr& cb,
                                                const string& pa, Int pos, Int num, const Current&) const
{
    if(isAbsolute(pa))
    {
        FileAccessException ex;
        ex.reason = "illegal absolute path `" + pa + "'";
        cb->ice_exception(ex);
        return;
    }

    string path = simplify(pa);
    
    if(path == ".." ||
       path.find("/../") != string::npos ||
       path.size() >= 3 && (path.substr(0, 3) == "../" || path.substr(path.size() - 3, 3) == "/.."))
    {
        FileAccessException ex;
        ex.reason = "illegal `..' component in path `" + path + "'";
        cb->ice_exception(ex);
        return;
    }

    pair<const Byte*, const Byte*> ret(0, 0);
    if(num <= 0 || pos < 0)
    {   
        cb->ice_response(ret);
        return;
    }

    int fd = OS::open(_dataDir + '/' + path + ".bz2", O_RDONLY|O_BINARY);
    if(fd == -1)
    {
        FileAccessException ex;
        ex.reason = "cannot open `" + path + "' for reading: " + strerror(errno);
        cb->ice_exception(ex);
        return;
    }

    if(lseek(fd, static_cast<off_t>(pos), SEEK_SET) != static_cast<off_t>(pos))
    {
        close(fd);

        ostringstream posStr;
        posStr << pos;

        FileAccessException ex;
        ex.reason = "cannot seek position " + posStr.str() + " in file `" + path + "': " + strerror(errno);
        cb->ice_exception(ex);
        return;
    }

    IceUtil::ScopedArray<Byte> bytes(new Byte[num]);
#ifdef _WIN32
    int r;
    if((r = read(fd, bytes.get(), static_cast<unsigned int>(num))) == -1)
#else
    ssize_t r;
    if((r = read(fd, bytes.get(), static_cast<size_t>(num))) == -1)
#endif
    {
        close(fd);

        FileAccessException ex;
        ex.reason = "cannot read `" + path + "': " + strerror(errno);
        cb->ice_exception(ex);
        return;
    }

    close(fd);

    ret.first = bytes.get();
    ret.second = ret.first + r;
    cb->ice_response(ret);
}
