// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/SliceChecksums.h>
#include <IceUtil/StaticMutex.h>

using namespace std;
using namespace Ice;

static SliceChecksumDict _sliceChecksums;
static IceUtil::StaticMutex _mutex = ICE_STATIC_MUTEX_INITIALIZER;

SliceChecksumDict
Ice::sliceChecksums()
{
    IceUtil::StaticMutex::Lock lock(_mutex);
    return _sliceChecksums;
}

IceInternal::SliceChecksumInit::SliceChecksumInit(const char* checksums[])
{
    IceUtil::StaticMutex::Lock lock(_mutex);
    for(int i = 0; checksums[i] != 0; i += 2)
    {
        _sliceChecksums.insert(SliceChecksumDict::value_type(checksums[i], checksums[i + 1]));
    }
}
