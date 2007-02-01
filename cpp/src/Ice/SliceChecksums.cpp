// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/SliceChecksums.h>
#include <IceUtil/StaticMutex.h>

using namespace std;
using namespace Ice;

static SliceChecksumDict* _sliceChecksums = 0;
static IceUtil::StaticMutex _mutex = ICE_STATIC_MUTEX_INITIALIZER;

class SliceChecksumDictDestroyer
{
public:
    
    ~SliceChecksumDictDestroyer()
    {
        delete _sliceChecksums;
        _sliceChecksums = 0;
    }
};
static SliceChecksumDictDestroyer destroyer;

SliceChecksumDict
Ice::sliceChecksums()
{
    IceUtil::StaticMutex::Lock lock(_mutex);
    if(_sliceChecksums == 0)
    {
        _sliceChecksums = new SliceChecksumDict();
    }
    return *_sliceChecksums;
}

IceInternal::SliceChecksumInit::SliceChecksumInit(const char* checksums[])
{
    IceUtil::StaticMutex::Lock lock(_mutex);
    if(_sliceChecksums == 0)
    {
        _sliceChecksums = new SliceChecksumDict();
    }

    for(int i = 0; checksums[i] != 0; i += 2)
    {
        _sliceChecksums->insert(SliceChecksumDict::value_type(checksums[i], checksums[i + 1]));
    }
}
