//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/DisableWarnings.h>
#include <AMDInterceptorI.h>
#include <MyObjectI.h>
#include <TestHelper.h>
#include <TestI.h>

using namespace std;

AMDInterceptorI::AMDInterceptorI(const Ice::ObjectPtr& servant) :
    InterceptorI(servant)
{
}

bool
AMDInterceptorI::dispatch(Ice::Request& request)
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

    if(_lastOperation == "amdAddWithRetry")
    {
        for(int i = 0; i < 10; ++i)
        {
            _lastStatus = _servant->ice_dispatch(request, nullptr, [](exception_ptr ex) {
                try
                {
                    rethrow_exception(ex);
                }
                catch(const Test::RetryException&)
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
        try
        {
            rethrow_exception(ex);
        }
        catch(const IceUtil::Exception& e)
        {
            setException(e);
        }
        catch(...)
        {
            test(false);
        }
        return true;
    });

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

void
AMDInterceptorI::setException(const IceUtil::Exception& e)
{
    IceUtil::Mutex::Lock lock(_mutex);
    _exception = e.ice_clone();
}

IceUtil::Exception*
AMDInterceptorI::getException() const
{
    IceUtil::Mutex::Lock lock(_mutex);
    return _exception.get();
}

void
AMDInterceptorI::clear()
{
    InterceptorI::clear();
    IceUtil::Mutex::Lock lock(_mutex);
    _exception.reset();
}
