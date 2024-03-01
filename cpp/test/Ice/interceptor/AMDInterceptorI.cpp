//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/DisableWarnings.h>
#include <AMDInterceptorI.h>
#include <MyObjectI.h>
#include <TestHelper.h>

using namespace std;
using namespace Test;
using namespace Ice;

AMDInterceptorI::AMDInterceptorI(const ObjectPtr& servant) :
    InterceptorI(servant)
{
}

bool
AMDInterceptorI::dispatch(Request& request)
{
    Current& current = const_cast<Current&>(request.getCurrent());

    Context::const_iterator p = current.ctx.find("raiseBeforeDispatch");
    if(p != current.ctx.end())
    {
        if(p->second == "user")
        {
            throw InvalidInputException();
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

    if(_lastOperation == "amdAddWithRetry")
    {
        for(int i = 0; i < 10; ++i)
        {
            _lastStatus = _servant->ice_dispatch(request, nullptr, [](exception_ptr ex) {
                try
                {
                    rethrow_exception(ex);
                }
                catch(const MyRetryException&)
                {
                }
                catch(...)
                {
                    test(false);
                }
                return false;
            });
            test(!_lastStatus);
        }

        current.ctx["retry"] = "no";
    }
    else if(current.ctx.find("retry") != current.ctx.end() && current.ctx["retry"] == "yes")
    {
        //
        // Retry the dispatch to ensure that abandoning the result of the dispatch
        // works fine and is thread-safe
        //
        _servant->ice_dispatch(request);
        _servant->ice_dispatch(request);
    }

    _lastStatus = _servant->ice_dispatch(request, []() { return true; }, [this](exception_ptr ex) {
        setException(ex);
        return true;
    });

    p = current.ctx.find("raiseAfterDispatch");
    if(p != current.ctx.end())
    {
        if(p->second == "user")
        {
            throw InvalidInputException();
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

void
AMDInterceptorI::setException(exception_ptr e)
{
    lock_guard lock(_mutex);
    _exception = e;
}

exception_ptr
AMDInterceptorI::getException() const
{
    lock_guard lock(_mutex);
    return _exception;
}

void
AMDInterceptorI::clear()
{
    InterceptorI::clear();
    lock_guard lock(_mutex);
    _exception = nullptr;
}
