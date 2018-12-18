// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <InterceptorI.h>
#include <Test.h>
#include <TestHelper.h>

using namespace std;

InterceptorI::InterceptorI(const Ice::ObjectPtr& servant) :
    _servant(servant),
    _lastStatus(false)
{
}

bool
InterceptorI::dispatch(Ice::Request& request)
{
    Ice::Current& current = const_cast<Ice::Current&>(request.getCurrent());
    _lastOperation = current.operation;

    if(_lastOperation == "addWithRetry")
    {
        for(int i = 0; i < 10; ++i)
        {
            try
            {
                _servant->ice_dispatch(request);
                test(false);
            }
            catch(const Test::RetryException&)
            {
                //
                // Expected, retry
                //
            }
        }

        current.ctx["retry"] = "no";
    }
    _lastStatus = _servant->ice_dispatch(request);
    return _lastStatus;
}

bool
InterceptorI::getLastStatus() const
{
    return _lastStatus;
}

const std::string&
InterceptorI::getLastOperation() const
{
    return _lastOperation;
}

void
InterceptorI::clear()
{
    _lastStatus = false;
    _lastOperation = "";
}
