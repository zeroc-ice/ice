// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/SliceChecksum.h>

using namespace std;
using namespace Ice;

const SliceChecksumDict Ice::sliceChecksums;

IceInternal::SliceChecksumInit::SliceChecksumInit(const char* checksums[])
{
    SliceChecksumDict& m = const_cast<SliceChecksumDict&>(sliceChecksums);
    for(int i = 0; checksums[i] != 0; i += 2)
    {
        m.insert(SliceChecksumDict::value_type(checksums[i], checksums[i + 1]));
    }
}
