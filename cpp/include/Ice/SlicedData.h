// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SLICED_DATA_H
#define ICE_SLICED_DATA_H

#include <Ice/SlicedDataF.h>
#include <Ice/GCObject.h>

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
class ICE_API SlicedData : public ::IceUtil::Shared
{
public:

    SlicedData(const SliceInfoSeq&);

    const SliceInfoSeq slices;

    void __gcVisitMembers(IceInternal::GCVisitor&);
};

//
// Unknown sliced object holds instance of unknown type.
//

#ifdef __IBMCPP__
// xlC does not handle properly the public/private multiple inheritance from Object
class ICE_API UnknownSlicedObject : public IceInternal::GCObject
#else
class ICE_API UnknownSlicedObject : virtual public Object, private IceInternal::GCObject
#endif
{
public:

    UnknownSlicedObject(const std::string&);

    const std::string& getUnknownTypeId() const;

    SlicedDataPtr getSlicedData() const;

    virtual void __gcVisitMembers(IceInternal::GCVisitor&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*);
    
    using Object::__write;
    using Object::__read;

private:

    const std::string _unknownTypeId;
    SlicedDataPtr _slicedData;
};

}

#endif
