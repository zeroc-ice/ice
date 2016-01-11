// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Value.h>
#include <Ice/Stream.h>
#include <Ice/LocalException.h>

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
Ice::Value::__write(IceInternal::BasicStream* os) const
{
    os->startWriteObject(0);
    __writeImpl(os);
    os->endWriteObject();
}

void
Ice::Value::__read(IceInternal::BasicStream* is)
{
   is->startReadObject();
   __readImpl(is);
   is->endReadObject(false);
}

namespace
{

const string __Ice__Object_ids[] =
{
    "::Ice::Object"
};

}

const string&
Ice::Value::ice_staticId()
{
    return __Ice__Object_ids[0];
}

const string&
Ice::Value::ice_id() const
{
    return __Ice__Object_ids[0];
}

ValuePtr
Ice::Value::ice_clone() const
{
    return cloneImpl();
}

#endif
