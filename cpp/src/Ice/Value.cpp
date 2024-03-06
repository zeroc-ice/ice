//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/Value.h>
#include <Ice/SlicedData.h>
#include <Ice/LocalException.h>

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

string
Ice::Value::ice_id() const
{
    return string{ice_staticId()};
}

string_view
Ice::Value::ice_staticId()
{
    static constexpr string_view typeId = "::Ice::Object";
    return typeId;
}

SlicedDataPtr
Ice::Value::ice_getSlicedData() const
{
    return _slicedData;
}

std::shared_ptr<Ice::Value>
Ice::Value::_iceCloneImpl() const
{
    throw CloneNotImplementedException(__FILE__, __LINE__);
}
