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

extern ICE_PATCH_API const std::string tmpName;

ICE_PATCH_API std::string identityToPath(const Ice::Identity&);
ICE_PATCH_API Ice::Identity pathToIdentity(const std::string&);

ICE_PATCH_API std::string pathToName(const std::string&);
ICE_PATCH_API std::string getSuffix(const std::string&);
ICE_PATCH_API std::string removeSuffix(const std::string&);

enum FileInfo
{
    FileInfoNotExist,
    FileInfoDirectory,
    FileInfoRegular,
    FileInfoUnknown
};
ICE_PATCH_API FileInfo getFileInfo(const std::string&);
ICE_PATCH_API void removeRecursive(const std::string&);

ICE_PATCH_API void changeDirectory(const std::string&);
ICE_PATCH_API Ice::StringSeq readDirectory(const std::string&);
ICE_PATCH_API void createDirectory(const std::string&);

ICE_PATCH_API Ice::ByteSeq getMD5(const std::string&);
ICE_PATCH_API void createMD5(const std::string&);
ICE_PATCH_API void createMD5Recursive(const std::string&);

ICE_PATCH_API Ice::Int getSizeBZ2(const std::string&);
ICE_PATCH_API Ice::ByteSeq getBytesBZ2(const std::string&, Ice::Int, Ice::Int);
ICE_PATCH_API void createBZ2(const std::string&);
ICE_PATCH_API void createBZ2Recursive(const std::string&);

ICE_PATCH_API void removeOrphanedRecursive(const std::string&);

class ICE_PATCH_API ProgressCB
{
public:

    virtual void start(Ice::Int) = 0;
    virtual void update(Ice::Int, Ice::Int) = 0;
    virtual void finished(Ice::Int) = 0;
};
ICE_PATCH_API void getFile(const IcePatch::FilePrx&, ProgressCB&);

}

#endif
