//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <InterceptorI.h>
#include <MyObjectI.h>
#include <TestHelper.h>
#include <TestI.h>

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

    Ice::Context::const_iterator p = current.ctx.find("raiseBeforeDispatch");
    if(p != current.ctx.end())
    {
        if(p->second == "user")
        {
            throw Test::InvalidInputException();
        }
        else if(p->second == "notExist")
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }
        else if(p->second == "system")
        {
            throw MySystemException(__FILE__, __LINE__);
        }
    }

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

    p = current.ctx.find("raiseAfterDispatch");
    if(p != current.ctx.end())
    {
        if(p->second == "user")
        {
            throw Test::InvalidInputException();
        }
        else if(p->second == "notExist")
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }
        else if(p->second == "system")
        {
            throw MySystemException(__FILE__, __LINE__);
        }
    }

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
