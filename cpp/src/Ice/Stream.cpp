// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Stream.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(InputStream* p) { return p; }
IceUtil::Shared* IceInternal::upCast(OutputStream* p) { return p; }

Ice::ReadObjectCallbackI::ReadObjectCallbackI(PatchFunc func, void* arg) :
    _func(func), _arg(arg)
{
}

void
Ice::ReadObjectCallbackI::invoke(const ::Ice::ObjectPtr& p)
{
    _func(_arg, const_cast< ::Ice::ObjectPtr& >(p));
}
