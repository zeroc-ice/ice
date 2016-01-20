// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
IceUtil::Shared* Ice::upCast(UnknownSlicedObject* p) { return p; }
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
        for(vector<ObjectPtr>::iterator q = (*p)->objects.begin(); q != (*p)->objects.end(); ++q)
        {
            if(q->get()->__gcVisit(visitor))
            {
                *q = 0;
            }
        }
    }
}

void
Ice::UnknownSlicedObject::__gcVisitMembers(IceInternal::GCVisitor& _v)
{
    if(_slicedData)
    {
        _slicedData->__gcVisitMembers(_v);
    }
}
#endif

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
Ice::UnknownSlicedObject::__write(Ice::OutputStream* __os) const
{
    __os->startObject(_slicedData);
    __os->endObject();
}

void
Ice::UnknownSlicedObject::__read(Ice::InputStream* __is)
{
    __is->startObject();
    _slicedData = __is->endObject(true);
}
