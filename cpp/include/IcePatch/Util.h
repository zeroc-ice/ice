// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PATCH_UTIL_H
#define ICE_PATCH_UTIL_H

#include <Ice/Ice.h>
#include <IcePatch/IcePatch.h>

namespace IcePatch
{

ICE_PATCH_API std::string identityToPath(const Ice::Identity&);
ICE_PATCH_API Ice::Identity pathToIdentity(const std::string&);

ICE_PATCH_API std::string getSuffix(const std::string&);
ICE_PATCH_API bool ignoreSuffix(const std::string&);
ICE_PATCH_API std::string removeSuffix(const std::string&);

enum FileType
{
    FileTypeNotExist,
    FileTypeDirectory,
    FileTypeRegular
};

struct FileInfo
{
    FileType type;
    off_t size;
    time_t time;
};

ICE_PATCH_API FileInfo getFileInfo(const std::string&, bool);

ICE_PATCH_API void removeRecursive(const std::string&);

ICE_PATCH_API Ice::StringSeq readDirectory(const std::string&);
ICE_PATCH_API void createDirectory(const std::string&);

ICE_PATCH_API Ice::ByteSeq getMD5(const std::string&);
ICE_PATCH_API void createMD5(const std::string&);
ICE_PATCH_API Ice::ByteSeq calcPartialMD5(const std::string&, Ice::Int);

ICE_PATCH_API Ice::ByteSeq getBZ2(const std::string&, Ice::Int, Ice::Int);
ICE_PATCH_API void createBZ2(const std::string&);

ICE_PATCH_API Ice::Long readStamp();
ICE_PATCH_API void writeStamp(Ice::Long stamp);

}

#endif
