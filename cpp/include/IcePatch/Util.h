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

namespace IcePatch
{

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
std::string MD5ToString(const Ice::ByteSeq&);

Ice::ByteSeq getBlockBZ2(const std::string&, Ice::Int);

}

#endif
