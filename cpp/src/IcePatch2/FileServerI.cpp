// **********************************************************************
//
// Copyright (c) 2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IcePatch2/FileServerI.h>

#ifdef _WIN32
#   include <io.h>
#else
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#   include <unistd.h>
#endif

using namespace std;
using namespace Ice;
using namespace IcePatch2;

IcePatch2::FileServerI::FileServerI(const FileInfoSeq& infoSeq)
{
    FileTree0& tree0 = const_cast<FileTree0&>(_tree0);
    getFileTree0(infoSeq, tree0);

/*
    assert(tree0.nodes.size() == 256);
    for(int i = 0; i < 256; ++i)
    {
	FileTree1& tree1 = tree0.nodes[i];
	assert(tree1.nodes.size() == 256);
	for(int j = 0; j < 256; ++j)
	{
	    FileTree2& tree2 = tree1.nodes[j];
	}
    }

    cout << "Summary ===> " << bytesToString(tree0.checksum) << endl;
*/
}

FileInfoSeq
IcePatch2::FileServerI::getFileInfoSeq(Int node0, Int node1, const Current&) const
{
    if(node0 < 0 || node0 > 255)
    {
	throw NodeOutOfRangeException();
    }

    if(node1 < 0 || node1 > 255)
    {
	throw NodeOutOfRangeException();
    }

    return _tree0.nodes[node0].nodes[node1].files;
}

ByteSeqSeq
IcePatch2::FileServerI::getChecksum2Seq(Int node0, const Current&) const
{
    if(node0 < 0 || node0 > 255)
    {
	throw NodeOutOfRangeException();
    }

    ByteSeqSeq checksums(256);

    for(int node1 = 0; node1 < 256; ++node1)
    {
	checksums[node1] = _tree0.nodes[node0].nodes[node1].checksum;
    }

    return checksums;
}

ByteSeqSeq
IcePatch2::FileServerI::getChecksum1Seq(const Current&) const
{
    ByteSeqSeq checksums(256);

    for(int node0 = 0; node0 < 256; ++node0)
    {
	checksums[node0] = _tree0.nodes[node0].checksum;
    }

    return checksums;
}

ByteSeq
IcePatch2::FileServerI::getChecksum0(const Current&) const
{
    return _tree0.checksum;
}

ByteSeq
IcePatch2::FileServerI::getFileCompressed(const string& pa, Int pos, Int num, const Current&) const
{
    string path = normalize(pa);
    path += ".bz2";

    if(num <= 0 || pos < 0)
    {
	return ByteSeq();
    }

    if(num > 256 * 1024)
    {
	num = 256 * 1024;
    }

    int fd = open(path.c_str(), O_RDONLY);
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
    ssize_t r;
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
