// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
