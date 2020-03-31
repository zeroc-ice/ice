//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/SlicedData.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>

using namespace std;
using namespace Ice;

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

SlicedDataPtr
Ice::UnknownSlicedValue::ice_getSlicedData() const
{
    return _slicedData;
}

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
