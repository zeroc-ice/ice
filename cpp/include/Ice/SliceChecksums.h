// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SLICE_CHECKSUM_H
#define ICE_SLICE_CHECKSUM_H

#include <Ice/SliceChecksumDict.h>

namespace Ice
{

/**
 * Obtains the map containing the checksums for Slice definitions.
 * @return The checksum map.
 */
ICE_API SliceChecksumDict sliceChecksums();

}

namespace IceInternal
{

class ICE_API SliceChecksumInit : private IceUtil::noncopyable
{
public:

    SliceChecksumInit(const char*[]);
};

}

#endif
