// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
Ice::Value::iceWrite(Ice::OutputStream* os) const
{
    os->startValue(0);
    iceWriteImpl(os);
    os->endValue();
}

void
Ice::Value::iceRead(Ice::InputStream* is)
{
   is->startValue();
   iceReadImpl(is);
   is->endValue(false);
}

namespace
{

const string iceC_Ice_Object_ids[] =
{
    "::Ice::Object"
};

}

string
Ice::Value::ice_id() const
{
    return iceC_Ice_Object_ids[0];
}

const string&
Ice::Value::ice_staticId()
{
    return iceC_Ice_Object_ids[0];
}

ValuePtr
Ice::Value::ice_clone() const
{
    return cloneImpl();
}

#endif
