//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Value.h>
#include <Ice/LocalException.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>

#ifdef ICE_CPP11_MAPPING

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
    os->startValue(0);
    _iceWriteImpl(os);
    os->endValue();
}

void
Ice::Value::_iceRead(Ice::InputStream* is)
{
   is->startValue();
   _iceReadImpl(is);
   is->endValue(false);
}

namespace
{

const string object_ids[] =
{
    "::Ice::Object"
};

}

string
Ice::Value::ice_id() const
{
    return object_ids[0];
}

const string&
Ice::Value::ice_staticId()
{
    return object_ids[0];
}

shared_ptr<Value>
Ice::Value::ice_clone() const
{
    return _iceCloneImpl();
}

shared_ptr<SlicedData>
Ice::Value::ice_getSlicedData() const
{
    return nullptr;
}

#endif
