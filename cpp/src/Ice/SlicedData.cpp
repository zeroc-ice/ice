// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/SlicedData.h>
#include <Ice/BasicStream.h>

using namespace std;
using namespace Ice;

IceUtil::Shared* Ice::upCast(SliceInfo* p) { return p; }
IceUtil::Shared* Ice::upCast(SlicedData* p) { return p; }
IceUtil::Shared* Ice::upCast(UnknownSlicedObject* p) { return p; }

Ice::SlicedData::SlicedData(const SliceInfoSeq& seq) :
    slices(seq)
{
}

void
Ice::SlicedData::__gcVisitMembers(IceInternal::GCVisitor& visitor)
{
    //
    // Iterate over the object references in each preserved slice.
    //
    for(SliceInfoSeq::const_iterator p = slices.begin(); p != slices.end(); ++p)
    {
        for(vector<ObjectPtr>::iterator q = (*p)->objects.begin(); q != (*p)->objects.end(); ++q)
        {
            if(q->get()->__gcVisit(visitor))
            {
                *q = 0;
            }
        }
    }
}

Ice::UnknownSlicedObject::UnknownSlicedObject(const string& unknownTypeId) : _unknownTypeId(unknownTypeId)
{
}

const string&
Ice::UnknownSlicedObject::getUnknownTypeId() const
{
    return _unknownTypeId;
}

SlicedDataPtr
Ice::UnknownSlicedObject::getSlicedData() const
{
    return _slicedData;
}

void
Ice::UnknownSlicedObject::__gcVisitMembers(IceInternal::GCVisitor& _v)
{
    if(_slicedData)
    {
        _slicedData->__gcVisitMembers(_v);
    }
}

void
Ice::UnknownSlicedObject::__write(IceInternal::BasicStream* __os) const
{
    __os->startWriteObject(_slicedData);
    __os->endWriteObject();
}

void
Ice::UnknownSlicedObject::__read(IceInternal::BasicStream* __is)
{
    __is->startReadObject();
    _slicedData = __is->endReadObject(true);
}
