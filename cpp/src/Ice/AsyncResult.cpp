// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CPP11_MAPPING

#include <IceUtil/DisableWarnings.h>
#include <Ice/AsyncResult.h>
#include <Ice/Proxy.h>

using namespace std;
using namespace Ice;

IceUtil::Shared* Ice::upCast(AsyncResult* p) { return p; }

AsyncResult::~AsyncResult()
{
    // Out of line to avoid weak vtable
}

void
AsyncResult::_check(const AsyncResultPtr& r, const IceProxy::Ice::Object* prx, const string& operation)
{
    check(r, operation);
    if(r->getProxy().get() != prx)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "Proxy for call to end_" + operation +
                                                " does not match proxy that was used to call corresponding begin_" +
                                                operation + " method");
    }
}

void
AsyncResult::_check(const AsyncResultPtr& r, const Ice::Communicator* com, const string& operation)
{
    check(r, operation);
    if(r->getCommunicator().get() != com)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "Communicator for call to end_" + operation +
                                                " does not match communicator that was used to call corresponding " +
                                                "begin_" + operation + " method");
    }
}

void
AsyncResult::_check(const AsyncResultPtr& r, const Ice::Connection* con, const string& operation)
{
    check(r, operation);
    if(r->getConnection().get() != con)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "Connection for call to end_" + operation +
                                                " does not match connection that was used to call corresponding " +
                                                "begin_" + operation + " method");
    }
}

void
AsyncResult::check(const AsyncResultPtr& r, const string& operation)
{
    if(!r)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "AsyncResult == null");
    }
    else if(r->getOperation() != operation)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "Incorrect operation for end_" + operation +
                                                " method: " + r->getOperation());
    }
}

#endif
