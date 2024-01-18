//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/SlicedData.h>

using namespace std;
using namespace Ice;

#ifndef ICE_CPP11_MAPPING

IceUtil::Shared* Ice::upCast(SliceInfo* p) { return p; }
IceUtil::Shared* Ice::upCast(SlicedData* p) { return p; }

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

Ice::UnknownSlicedValue::UnknownSlicedValue(const string& unknownTypeId) : _unknownTypeId(unknownTypeId)
{
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

string
Ice::UnknownSlicedValue::ice_id() const
{
    return _unknownTypeId;
}

#endif
