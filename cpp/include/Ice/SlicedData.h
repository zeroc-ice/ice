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
#include <Ice/Value.h>

namespace Ice
{

//
// SliceInfo encapsulates the details of a slice for an unknown class or exception type.
//
struct ICE_API SliceInfo
#ifndef ICE_CPP11_MAPPING
    : public ::IceUtil::Shared
#endif
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
    // The class instances referenced by this slice.
    //
    ::std::vector<ValuePtr> instances;

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
class ICE_API SlicedData
#ifndef ICE_CPP11_MAPPING
    : public ::IceUtil::Shared
#endif
{
public:

#ifndef ICE_CPP11_MAPPING
    virtual ~SlicedData();
#endif

    SlicedData(const SliceInfoSeq&);

    const SliceInfoSeq slices;
#ifndef ICE_CPP11_MAPPING
    void _iceGcVisitMembers(IceInternal::GCVisitor&);
#endif
};

//
// Unknown sliced object holds instance of unknown type.
//
class ICE_API UnknownSlicedValue :
#ifdef ICE_CPP11_MAPPING
    public Value
#else
    public IceInternal::GCObject
#endif
{
public:

    UnknownSlicedValue(const std::string&);

    const std::string& getUnknownTypeId() const;

    SlicedDataPtr getSlicedData() const;

#ifdef ICE_CPP11_MAPPING
    virtual void _iceWrite(::Ice::OutputStream*) const override;
    virtual void _iceRead(::Ice::InputStream*) override;

    virtual std::string ice_id() const override;
    std::shared_ptr<UnknownSlicedValue> ice_clone() const;

protected:

    virtual std::shared_ptr<Value> cloneImpl() const override;
#else
    virtual void _iceGcVisitMembers(IceInternal::GCVisitor&);

    virtual void _iceWrite(::Ice::OutputStream*) const;
    virtual void _iceRead(::Ice::InputStream*);
#endif

private:

    const std::string _unknownTypeId;
    SlicedDataPtr _slicedData;
};

}

#endif
