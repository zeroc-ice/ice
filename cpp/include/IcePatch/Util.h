// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PATCH_UTIL_H
#define ICE_PATCH_UTIL_H

#include <Ice/Ice.h>
#include <IcePatch/Node.h>

namespace IcePatch
{

extern const std::string tmpName;

std::string identityToPath(const Ice::Identity&);
Ice::Identity pathToIdentity(const std::string&);

std::string pathToName(const std::string&);
std::string getSuffix(const std::string&);

enum FileInfo
{
    FileInfoNotExist,
    FileInfoDirectory,
    FileInfoRegular,
    FileInfoUnknown
};
FileInfo getFileInfo(const std::string&);
Ice::StringSeq readDirectory(const std::string&);
void removeRecursive(const std::string&);
void createDirectory(const std::string&);

Ice::ByteSeq getMD5(const std::string&);
void createMD5(const std::string&);
void createMD5Recursive(const std::string&);

void writeBZ2(const std::string&, const Ice::ByteSeq&);
Ice::ByteSeq readBZ2(const std::string&);

Ice::Int getSizeBZ2(const std::string&);
Ice::ByteSeq getBytesBZ2(const std::string&, Ice::Int, Ice::Int);
void createBZ2(const std::string&);
void createBZ2Recursive(const std::string&);

class ProgressCB
{
public:

    virtual void start(Ice::Int) = 0;
    virtual void update(Ice::Int, Ice::Int) = 0;
    virtual void finished(Ice::Int) = 0;
};
void getFile(const IcePatch::FilePrx&, ProgressCB&);

}

#endif
