//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Value.h"
#include "Ice/Demangle.h"
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
Ice::Value::ice_print(ostream& os) const
{
    os << demangle(typeid(*this).name()) << "{";
    ice_printFields(os);
    os << "}";
}

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
