// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_FILE_CACHE_H
#define ICE_GRID_FILE_CACHE_H

#include <IceUtil/Shared.h>
#include <Ice/BuiltinSequences.h>

namespace IceGrid
{

class FileCache : public IceUtil::Shared
{
public:

    FileCache();

    Ice::Long getOffsetFromEnd(const std::string&, int);
    bool read(const std::string&, Ice::Long, int, int, Ice::Long&, Ice::StringSeq&);
};
typedef IceUtil::Handle<FileCache> FileCachePtr;

};

#endif
