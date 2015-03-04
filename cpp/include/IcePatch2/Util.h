// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PATCH2_UTIL_H
#define ICE_PATCH2_UTIL_H

#include <Ice/Ice.h>
#include <IcePatch2/FileInfo.h>
#include <stdio.h>

namespace IcePatch2
{

ICE_PATCH2_API extern const char* checksumFile;
ICE_PATCH2_API extern const char* logFile;

ICE_PATCH2_API std::string lastError();

ICE_PATCH2_API std::string bytesToString(const Ice::ByteSeq&);
ICE_PATCH2_API Ice::ByteSeq stringToBytes(const std::string&);

ICE_PATCH2_API std::string simplify(const std::string&);

ICE_PATCH2_API bool isRoot(const std::string&);

ICE_PATCH2_API std::string getSuffix(const std::string&);
ICE_PATCH2_API std::string getWithoutSuffix(const std::string&);
ICE_PATCH2_API bool ignoreSuffix(const std::string&);

ICE_PATCH2_API std::string getBasename(const std::string&);
ICE_PATCH2_API std::string getDirname(const std::string&);

ICE_PATCH2_API void rename(const std::string&, const std::string&);

ICE_PATCH2_API void remove(const std::string&);
ICE_PATCH2_API void removeRecursive(const std::string&);

ICE_PATCH2_API Ice::StringSeq readDirectory(const std::string&);

ICE_PATCH2_API void createDirectory(const std::string&);
ICE_PATCH2_API void createDirectoryRecursive(const std::string&);

ICE_PATCH2_API void compressBytesToFile(const std::string&, const Ice::ByteSeq&, Ice::Int);
ICE_PATCH2_API void decompressFile(const std::string&);
ICE_PATCH2_API void setFileFlags(const std::string&, const FileInfo&);

struct FileInfoEqual: public std::binary_function<const FileInfo&, const FileInfo&, bool>
{
    bool
    operator()(const FileInfo& lhs, const FileInfo& rhs)
    {
        if(lhs.path != rhs.path)
        {
            return false;
        }

        //
        // For the size portion of the comparison, we only distinquish
        // between file (size >= 0) and directory (size == -1). We do
        // not take the actual size into account, as it might be set
        // to 0 if no compressed file is available.
        //
        Ice::Int lsz = lhs.size > 0 ? 0 : lhs.size;
        Ice::Int rsz = rhs.size > 0 ? 0 : rhs.size;
        if(lsz != rsz)
        {
            return false;
        }

        if(lhs.executable != rhs.executable)
        {
            return false;
        }
        
        return lhs.checksum == rhs.checksum;
    }
};

struct FileInfoWithoutFlagsLess: public std::binary_function<const FileInfo&, const FileInfo&, bool>
{
    bool
    operator()(const FileInfo& lhs, const FileInfo& rhs)
    {
        return compareWithoutFlags(lhs, rhs) < 0;
    }

    int
    compareWithoutFlags(const FileInfo& lhs, const FileInfo& rhs)
    {
        if(lhs.path < rhs.path)
        {
            return -1;
        }
        else if(rhs.path < lhs.path)
        {
            return 1;
        }

        //
        // For the size portion of the comparison, we only distinquish
        // between file (size >= 0) and directory (size == -1). We do
        // not take the actual size into account, as it might be set
        // to 0 if no compressed file is available.
        //
        Ice::Int lsz = lhs.size > 0 ? 0 : lhs.size;
        Ice::Int rsz = rhs.size > 0 ? 0 : rhs.size;
        if(lsz < rsz)
        {
            return -1;
        }
        else if(rsz < lsz)
        {
            return 1;
        }

        if(lhs.checksum < rhs.checksum)
        {
            return -1;
        }
        else if(rhs.checksum < lhs.checksum)
        {
            return 1;
        }
        
        return 0;
    }
};

struct FileInfoLess : public FileInfoWithoutFlagsLess
{
    bool
    operator()(const FileInfo& lhs, const FileInfo& rhs)
    {
        int rc = compareWithoutFlags(lhs, rhs);
        if(rc < 0)
        {
            return true;
        }
        else if(rc > 0)
        {
            return false;
        }

        return lhs.executable < rhs.executable;
    }
};

class ICE_PATCH2_API GetFileInfoSeqCB
{
public:

    virtual ~GetFileInfoSeqCB() { }

    virtual bool remove(const std::string&) = 0;
    virtual bool checksum(const std::string&) = 0;
    virtual bool compress(const std::string&) = 0;
};

ICE_PATCH2_API bool getFileInfoSeq(const std::string&, int, GetFileInfoSeqCB*, FileInfoSeq&);
ICE_PATCH2_API bool getFileInfoSeqSubDir(const std::string&, const std::string&, int, GetFileInfoSeqCB*, FileInfoSeq&);

ICE_PATCH2_API void saveFileInfoSeq(const std::string&, const FileInfoSeq&);
ICE_PATCH2_API void loadFileInfoSeq(const std::string&, FileInfoSeq&);

ICE_PATCH2_API bool readFileInfo(FILE*, FileInfo&);
ICE_PATCH2_API bool writeFileInfo(FILE*, const FileInfo&);

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

ICE_PATCH2_API void getFileTree0(const FileInfoSeq&, FileTree0&);

}

#endif
