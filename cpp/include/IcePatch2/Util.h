// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PATCH2_UTIL_H
#define ICE_PATCH2_UTIL_H

#include <Ice/Ice.h>
#include <IcePatch2/FileInfo.h>

namespace IcePatch2
{

ICE_PATCH2_API std::string lastError();

ICE_PATCH2_API std::string bytesToString(const Ice::ByteSeq&);
ICE_PATCH2_API Ice::ByteSeq stringToBytes(const std::string&);

ICE_PATCH2_API std::string normalize(const std::string&);

ICE_PATCH2_API std::string getSuffix(const std::string&);
ICE_PATCH2_API std::string getWithoutSuffix(const std::string&);
ICE_PATCH2_API bool ignoreSuffix(const std::string&);

ICE_PATCH2_API std::string getBasename(const std::string&);
ICE_PATCH2_API std::string getDirname(const std::string&);

ICE_PATCH2_API void remove(const std::string&);
ICE_PATCH2_API void removeRecursive(const std::string&);

ICE_PATCH2_API Ice::StringSeq readDirectory(const std::string&);
ICE_PATCH2_API void createDirectoryRecursive(const std::string&);

ICE_PATCH2_API void compressBytesToFile(const std::string&, const Ice::ByteSeq&, Ice::Int);
ICE_PATCH2_API void decompressFile(const std::string&);

struct FileInfoEqual: public std::binary_function<const FileInfo&, const FileInfo&, bool>
{
    bool
    operator()(const FileInfo& lhs, const FileInfo& rhs)
    {
	return lhs.path == rhs.path && lhs.checksum < rhs.checksum;

	//
	// We don't take the size int account, as it might not be set
	// if no compressed file is available.
	//
    }
};

struct FileInfoLess: public std::binary_function<const FileInfo&, const FileInfo&, bool>
{
    bool
    operator()(const FileInfo& lhs, const FileInfo& rhs)
    {
	if(lhs.path < rhs.path)
	{
	    return true;
	}
	else if(rhs.path < lhs.path)
	{
	    return false;
	}

	return lhs.checksum < rhs.checksum;

	//
	// We don't take the size int account, as it might not be set
	// if no compressed file is available.
	//
    }
};

ICE_PATCH2_API void getFileInfoSeq(const std::string&, FileInfoSeq&, bool, bool, bool);
ICE_PATCH2_API void saveFileInfoSeq(const std::string&, const FileInfoSeq&);
ICE_PATCH2_API void loadFileInfoSeq(const std::string&, FileInfoSeq&);

ICE_PATCH2_API std::ostream& operator<<(std::ostream&, const FileInfo&);
ICE_PATCH2_API std::istream& operator>>(std::istream&, FileInfo&);

struct FileTree1
{
    FileInfoSeq files;
    Ice::ByteSeq checksum;
};

typedef std::vector<FileTree1> FileTree1Seq;

struct FileTree0
{
    FileTree1Seq nodes;
    Ice::ByteSeq checksum;
};

ICE_PATCH2_API void getFileTree1(const FileInfoSeq&, FileTree1&);
ICE_PATCH2_API void getFileTree0(const FileInfoSeq&, FileTree0&);

}

#endif
