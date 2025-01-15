//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Value.h"
#include "Ice/InputStream.h"
#include "Ice/LocalExceptions.h"
#include "Ice/OutputStream.h"
#include "Ice/SlicedData.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
Ice::Value::ice_preMarshal()
{
}

void
Ice::Value::ice_postUnmarshal()
{
}

void
Ice::Value::ice_print(ostream& os, std::deque<const Value*>*) const
{
    // This implementation must be used only for classes that can't be cyclic (such as Ice::Object).
    string typeId{ice_id()};
    os << typeId.substr(2) << " { ";
    ice_printFields(os, nullptr);
    os << " }";
}

void
Ice::Value::ice_printFields(ostream&, std::deque<const Value*>*) const {}

void
Ice::Value::_iceWrite(Ice::OutputStream* os) const
{
    os->startValue(_slicedData);
    _iceWriteImpl(os);
    os->endValue();
}

void
Ice::Value::_iceRead(Ice::InputStream* is)
{
    is->startValue();
    _iceReadImpl(is);
    _slicedData = is->endValue();
}

const char*
Ice::Value::ice_id() const noexcept
{
    return ice_staticId();
}

const char*
Ice::Value::ice_staticId() noexcept
{
    return "::Ice::Object";
}

SlicedDataPtr
Ice::Value::ice_getSlicedData() const
{
    return _slicedData;
}

Ice::ValuePtr
Ice::Value::_iceCloneImpl() const
{
    throw std::logic_error("clone is not implemented for this class");
}
