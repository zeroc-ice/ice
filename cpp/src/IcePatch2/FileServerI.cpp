// **********************************************************************
//
// Copyright (c) 2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IcePatch2/FileServerI.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef _WIN32
#   include <io.h>
#else
#   include <unistd.h>
#endif

using namespace std;
using namespace Ice;
using namespace IcePatch2;

IcePatch2::FileServerI::FileServerI(const CommunicatorPtr& communicator,
				    const std::string& dataDir,
				    const FileInfoSeq& infoSeq) :
    _dataDir(normalize(dataDir)),
    _dataDirWithSlash(_dataDir + "/")
{
    int sizeMax = communicator->getProperties()->getPropertyAsIntWithDefault("Ice.MessageSizeMax", 1024);
    _maxReadSize = communicator->getProperties()->getPropertyAsIntWithDefault("IcePatch2.MaxReadSize", 256);
    if(_maxReadSize < 1)
    {
        _maxReadSize = 1;
    }
    else if(_maxReadSize > sizeMax)
    {
        _maxReadSize = sizeMax;
    }
    if(_maxReadSize == sizeMax)
    {
       _maxReadSize = _maxReadSize * 1024 - 512; // Leave some headroom for protocol header.
    }
    else
    {
	_maxReadSize *= 1024;
    }
    FileTree0& tree0 = const_cast<FileTree0&>(_tree0);
    getFileTree0(infoSeq, tree0);
}

FileInfoSeq
IcePatch2::FileServerI::getFileInfoSeq(Int partition, const Current&) const
{
    if(partition < 0 || partition > 255)
    {
	throw PartitionOutOfRangeException();
    }

    return _tree0.nodes[partition].files;
}

ByteSeqSeq
IcePatch2::FileServerI::getChecksumSeq(const Current&) const
{
    ByteSeqSeq checksums(NumPartitions);

    for(int part = 0; part < NumPartitions; ++part)
    {
	checksums[part] = _tree0.nodes[part].checksum;
    }

    return checksums;
}

ByteSeq
IcePatch2::FileServerI::getChecksum(const Current&) const
{
    return _tree0.checksum;
}

ByteSeq
IcePatch2::FileServerI::getFileCompressed(const string& pa, Int pos, Int num, const Current&) const
{
    string path = normalize(pa) + ".bz2";
    if(path.compare(0, _dataDirWithSlash.size(), _dataDirWithSlash) != 0)
    {
	FileAccessException ex;
	ex.reason = "`" + pa + "' is not a path in `" + _dataDir + "'";
	throw ex;
    }

    if(num <= 0 || pos < 0)
    {
	return ByteSeq();
    }

    if(num > _maxReadSize)
    {
	num = _maxReadSize;
    }

#ifdef _WIN32
    int fd = open(path.c_str(), _O_RDONLY | _O_BINARY);
#else
    int fd = open(path.c_str(), O_RDONLY);
#endif
    if(fd == -1)
    {
	FileAccessException ex;
	ex.reason = "cannot open `" + path + "' for reading: " + strerror(errno);
	throw ex;
    }

    if(lseek(fd, static_cast<off_t>(pos), SEEK_SET) != static_cast<off_t>(pos))
    {
	close(fd);

	ostringstream posStr;
	posStr << pos;

	FileAccessException ex;
	ex.reason = "cannot seek position " + posStr.str() + " in file `" + path + "': " + strerror(errno);
	throw ex;
    }

    ByteSeq bytes(num);
#ifdef _WIN32
    long r;
#else
    ssize_t r;
#endif
    if((r = read(fd, &bytes[0], static_cast<size_t>(num))) == -1)
    {
	close(fd);

	FileAccessException ex;
	ex.reason = "cannot read `" + path + "': " + strerror(errno);
	throw ex;
    }

    close(fd);

    bytes.resize(r);
    return bytes;
}
