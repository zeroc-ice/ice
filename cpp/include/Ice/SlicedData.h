// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SLICED_DATA_H
#define ICE_SLICED_DATA_H

#include <Ice/SlicedDataF.h>
#include <Ice/Object.h>

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
    // The Slice compact type ID for this slice.
    //
    int compactId;

    //
    // The encoded bytes for this slice, including the leading size integer.
    //
    ::std::vector<Byte> bytes;

    //
    // The Ice objects referenced by this slice.
    //
    ::std::vector<ObjectPtr> objects;

    //
    // Whether or not the slice contains optional members.
    //
    bool hasOptionalMembers;

    //
    // Whether or not this is the last slice.
    //
    bool isLastSlice;
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

//
// Unknown sliced object holds instance of unknown type.
//
class ICE_API UnknownSlicedObject : public Object, private IceInternal::GCShared 
{
public:

    UnknownSlicedObject(const std::string&);

    const std::string& getUnknownTypeId() const;

    SlicedDataPtr getSlicedData() const;

    virtual void __addObject(::IceInternal::GCCountMap&);
    virtual bool __usesGC();
    virtual void __gcReachable(::IceInternal::GCCountMap&) const;
    virtual void __gcClear();

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*);
    
#ifdef __SUNPRO_CC
    using Object::__write;
    using Object::__read;
#endif

private:

    const std::string _unknownTypeId;
    SlicedDataPtr _slicedData;
};

}

#endif
