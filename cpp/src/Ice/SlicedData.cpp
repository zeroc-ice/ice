// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/SlicedData.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>

using namespace std;
using namespace Ice;

#ifndef ICE_CPP11_MAPPING
IceUtil::Shared* Ice::upCast(SliceInfo* p) { return p; }
IceUtil::Shared* Ice::upCast(SlicedData* p) { return p; }
IceUtil::Shared* Ice::upCast(UnknownSlicedValue* p) { return p; }

Ice::SlicedData::~SlicedData()
{
    // Out of line to avoid weak vtable
}
#endif

Ice::SlicedData::SlicedData(const SliceInfoSeq& seq) :
    slices(seq)
{
}


#ifndef ICE_CPP11_MAPPING
void
Ice::SlicedData::__gcVisitMembers(IceInternal::GCVisitor& visitor)
{
    //
    // Iterate over the object references in each preserved slice.
    //
    for(SliceInfoSeq::const_iterator p = slices.begin(); p != slices.end(); ++p)
    {
        for(vector<ObjectPtr>::iterator q = (*p)->instances.begin(); q != (*p)->instances.end(); ++q)
        {
            if(q->get()->__gcVisit(visitor))
            {
                *q = 0;
            }
        }
    }
}

void
Ice::UnknownSlicedValue::__gcVisitMembers(IceInternal::GCVisitor& _v)
{
    if(_slicedData)
    {
        _slicedData->__gcVisitMembers(_v);
    }
}
#endif

Ice::UnknownSlicedValue::UnknownSlicedValue(const string& unknownTypeId) : _unknownTypeId(unknownTypeId)
{
}

const string&
Ice::UnknownSlicedValue::getUnknownTypeId() const
{
    return _unknownTypeId;
}

SlicedDataPtr
Ice::UnknownSlicedValue::getSlicedData() const
{
    return _slicedData;
}

void
Ice::UnknownSlicedValue::__write(Ice::OutputStream* __os) const
{
    __os->startValue(_slicedData);
    __os->endValue();
}

void
Ice::UnknownSlicedValue::__read(Ice::InputStream* __is)
{
    __is->startValue();
    _slicedData = __is->endValue(true);
}
