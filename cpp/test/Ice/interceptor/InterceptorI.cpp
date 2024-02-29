//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <InterceptorI.h>
#include <MyObjectI.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;

InterceptorI::InterceptorI(const ObjectPtr& servant) :
    _servant(servant),
    _lastStatus(false)
{
}

bool
InterceptorI::dispatch(Request& request)
{
    Current& current = const_cast<Current&>(request.getCurrent());

    Context::const_iterator p = current.ctx.find("raiseBeforeDispatch");
    if(p != current.ctx.end())
    {
        if(p->second == "user")
        {
            throw Test::InvalidInputException();
        }
        else if(p->second == "notExist")
        {
            throw ObjectNotExistException(__FILE__, __LINE__);
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
            catch(const MyRetryException&)
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
            throw ObjectNotExistException(__FILE__, __LINE__);
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

const string&
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
