// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_FILE_CACHE_H
#define ICE_GRID_FILE_CACHE_H

#include <IceUtil/Shared.h>
#include <Ice/BuiltinSequences.h>
#include <Ice/CommunicatorF.h>

namespace IceGrid
{

class FileCache : public IceUtil::Shared
{
public:

    FileCache(const Ice::CommunicatorPtr&);

    Ice::Long getOffsetFromEnd(const std::string&, int);
    bool read(const std::string&, Ice::Long, int, Ice::Long&, Ice::StringSeq&);

private:

    const int _messageSizeMax;
};
typedef IceUtil::Handle<FileCache> FileCachePtr;

};

#endif
