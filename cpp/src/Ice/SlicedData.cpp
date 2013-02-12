// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/SlicedData.h>
#include <Ice/Object.h>
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
Ice::SlicedData::__gcReachable(IceInternal::GCCountMap& m) const
{
    //
    // Iterate over the object references in each preserved slice.
    //
    for(SliceInfoSeq::const_iterator p = slices.begin(); p != slices.end(); ++p)
    {
        for(vector<ObjectPtr>::const_iterator q = (*p)->objects.begin(); q != (*p)->objects.end(); ++q)
        {
            (*q)->__addObject(m);
        }
    }
}

void
Ice::SlicedData::__gcClear()
{
    //
    // Iterate over the object references in each preserved slice.
    //
    for(SliceInfoSeq::const_iterator p = slices.begin(); p != slices.end(); ++p)
    {
        for(vector<ObjectPtr>::iterator q = (*p)->objects.begin(); q != (*p)->objects.end(); ++q)
        {
            if((*q)->__usesGC())
            {
                (*q)->__decRefUnsafe();
                (*q).__clearHandleUnsafe();
            }
        }
        (*p)->objects.clear();
    }
}

void
Ice::SlicedData::__addObject(IceInternal::GCCountMap& m)
{
    IceInternal::GCCountMap::iterator pos = m.find(this);
    if(pos == m.end())
    {
        m[this] = 1;
    }
    else
    {
        ++pos->second;
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
Ice::UnknownSlicedObject::__addObject(IceInternal::GCCountMap& _c)
{
    IceInternal::GCCountMap::iterator pos = _c.find(this);
    if(pos == _c.end())
    {
        _c[this] = 1;
    }
    else
    {
        ++pos->second;
    }
}

bool
Ice::UnknownSlicedObject::__usesGC()
{
    return true;
}

void
Ice::UnknownSlicedObject::__gcReachable(IceInternal::GCCountMap& _c) const
{
    if(_slicedData)
    {
        _slicedData->__addObject(_c);
    }
}

void
Ice::UnknownSlicedObject::__gcClear()
{
    if(_slicedData)
    {
        _slicedData->__decRefUnsafe();
        _slicedData.__clearHandleUnsafe();
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
