// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Stream.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(InputStream* p) { p->__incRef(); }
void IceInternal::decRef(InputStream* p) { p->__decRef(); }
void IceInternal::incRef(OutputStream* p) { p->__incRef(); }
void IceInternal::decRef(OutputStream* p) { p->__decRef(); }

Ice::ReadObjectCallbackI::ReadObjectCallbackI(PatchFunc func, void* arg) :
    _func(func), _arg(arg)
{
}

void
Ice::ReadObjectCallbackI::invoke(const ::Ice::ObjectPtr& p)
{
    _func(_arg, const_cast< ::Ice::ObjectPtr& >(p));
}
