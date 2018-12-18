// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

void
Ice::SlicedData::clear()
{
    SliceInfoSeq tmp;
    tmp.swap(const_cast<SliceInfoSeq&>(slices));
    for(SliceInfoSeq::const_iterator p = tmp.begin(); p != tmp.end(); ++p)
    {
        for(vector<ValuePtr>::const_iterator q = (*p)->instances.begin(); q != (*p)->instances.end(); ++q)
        {
            Ice::SlicedDataPtr slicedData = (*q)->ice_getSlicedData();
            if(slicedData)
            {
                slicedData->clear();
            }
        }
    }
}

#ifndef ICE_CPP11_MAPPING
void
Ice::SlicedData::_iceGcVisitMembers(IceInternal::GCVisitor& visitor)
{
    //
    // Iterate over the object references in each preserved slice.
    //
    for(SliceInfoSeq::const_iterator p = slices.begin(); p != slices.end(); ++p)
    {
        for(vector<ObjectPtr>::iterator q = (*p)->instances.begin(); q != (*p)->instances.end(); ++q)
        {
            if(q->get()->_iceGcVisit(visitor))
            {
                *q = 0;
            }
        }
    }
}
#endif

Ice::UnknownSlicedValue::UnknownSlicedValue(const string& unknownTypeId) : _unknownTypeId(unknownTypeId)
{
}

SlicedDataPtr
Ice::UnknownSlicedValue::ice_getSlicedData() const
{
    return _slicedData;
}

#ifdef ICE_CPP11_MAPPING

string
Ice::UnknownSlicedValue::ice_id() const
{
    return _unknownTypeId;
}

shared_ptr<Ice::UnknownSlicedValue>
Ice::UnknownSlicedValue::ice_clone() const
{
    return static_pointer_cast<UnknownSlicedValue>(_iceCloneImpl());
}

shared_ptr<Ice::Value>
Ice::UnknownSlicedValue::_iceCloneImpl() const
{
    return make_shared<UnknownSlicedValue>(static_cast<const UnknownSlicedValue&>(*this));
}

#else

const string&
Ice::UnknownSlicedValue::ice_id(const Current&) const
{
    return _unknownTypeId;
}

void
Ice::UnknownSlicedValue::_iceGcVisitMembers(IceInternal::GCVisitor& _v)
{
    if(_slicedData)
    {
        _slicedData->_iceGcVisitMembers(_v);
    }
}

#endif

void
Ice::UnknownSlicedValue::_iceWrite(Ice::OutputStream* ostr) const
{
    ostr->startValue(_slicedData);
    ostr->endValue();
}

void
Ice::UnknownSlicedValue::_iceRead(Ice::InputStream* istr)
{
    istr->startValue();
    _slicedData = istr->endValue(true);
}
