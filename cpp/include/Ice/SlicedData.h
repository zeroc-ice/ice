// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SLICED_DATA_H
#define ICE_SLICED_DATA_H

#include <Ice/SlicedDataF.h>
#include <Ice/ObjectF.h>
#include <Ice/GCShared.h>

namespace Ice
{

//
// SliceInfo encapsulates the details of a slice for an unknown class or exception type.
//
struct ICE_API SliceInfo : public ::IceUtil::Shared
{
    //
    // The Slice type ID for this slice.
    //
    ::std::string typeId;

    //
    // The encoded bytes for this slice, including the leading size integer.
    //
    ::std::vector<Byte> bytes;

    //
    // The Ice objects referenced by this slice.
    //
    ::std::vector<ObjectPtr> objects;
};

//
// SlicedData holds the slices of unknown types.
//
class ICE_API SlicedData : public ::IceInternal::GCShared
{
public:

    SlicedData(const SliceInfoSeq&);

    const SliceInfoSeq slices;

    //
    // The internal methods below are necessary to support garbage collection
    // of Ice objects.
    //

    virtual void __gcReachable(IceInternal::GCCountMap&) const;
    virtual void __gcClear();

    void __decRefUnsafe()
    {
        --_ref;
    }

    void __addObject(IceInternal::GCCountMap&);
};

}

#endif
