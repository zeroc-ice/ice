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
namespace
{

const string iceObject = "::Ice::Object";

}

#ifdef ICE_CPP11_MAPPING

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
    os->startValue(_iceSlicedData);
    _iceWriteImpl(os);
    os->endValue();
}

void
Ice::Value::_iceRead(Ice::InputStream* is)
{
   is->startValue();
   _iceReadImpl(is);
   _iceSlicedData = is->endValue();
}

string
Ice::Value::ice_id() const
{
    return ice_staticId();
}

const string&
Ice::Value::ice_staticId()
{
    return iceObject;
}

shared_ptr<Value>
Ice::Value::ice_clone() const
{
    return _iceCloneImpl();
}

shared_ptr<SlicedData>
Ice::Value::ice_getSlicedData() const
{
    return _iceSlicedData;
}

#else

void
Ice::_icePatchValuePtr(ValuePtr& obj, const ValuePtr& v)
{
    obj = v;
}

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
    os->startValue(_iceSlicedData);
    _iceWriteImpl(os);
    os->endValue();
}

void
Ice::Value::_iceRead(Ice::InputStream* is)
{
   is->startValue();
   _iceReadImpl(is);
   _iceSlicedData = is->endValue();
}

string
Ice::Value::ice_id() const
{
    return ice_staticId();
}

const string&
Ice::Value::ice_staticId()
{
    return iceObject;
}

Ice::ValuePtr
Ice::Value::ice_clone() const
{
    throw CloneNotImplementedException(__FILE__, __LINE__);
}

Ice::SlicedDataPtr
Ice::Value::ice_getSlicedData() const
{
    return _iceSlicedData;
}

#endif
