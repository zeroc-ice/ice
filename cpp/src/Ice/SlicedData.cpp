// Copyright (c) ZeroC, Inc.

#include "Ice/SlicedData.h"
#include "Ice/InputStream.h"
#include "Ice/OutputStream.h"

using namespace std;
using namespace Ice;

Ice::SlicedData::SlicedData(SliceInfoSeq seq) noexcept : slices(std::move(seq)) {}

void
Ice::SlicedData::clear()
{
    SliceInfoSeq tmp;
    tmp.swap(const_cast<SliceInfoSeq&>(slices));
    for (const auto& p : tmp)
    {
        for (const auto& instance : p->instances)
        {
            Ice::SlicedDataPtr slicedData = instance->ice_getSlicedData();
            if (slicedData)
            {
                slicedData->clear();
            }
        }
    }
}

Ice::UnknownSlicedValue::UnknownSlicedValue(string unknownTypeId) noexcept : _unknownTypeId(std::move(unknownTypeId)) {}

const char*
Ice::UnknownSlicedValue::ice_id() const noexcept
{
    return _unknownTypeId.c_str();
}

void
Ice::UnknownSlicedValue::ice_printFields(ostream& os) const
{
    os << "unknownTypeId = " << _unknownTypeId;
}

ValuePtr
Ice::UnknownSlicedValue::_iceCloneImpl() const
{
    return make_shared<UnknownSlicedValue>(static_cast<const UnknownSlicedValue&>(*this));
}
