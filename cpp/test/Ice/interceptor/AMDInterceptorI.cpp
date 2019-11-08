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
#ifndef ICE_CPP11_MAPPING
    , _defaultCb(new DispatchInterceptorAsyncCallbackI(*this))
#endif
{
}

bool
AMDInterceptorI::dispatch(Ice::Request& request)
{
#ifndef ICE_CPP11_MAPPING
    class CallbackI : public Ice::DispatchInterceptorAsyncCallback
    {
    public:
        CallbackI() : _count(0)
        {
        }

        virtual bool response()
        {
            return false;
        }

        virtual bool exception(const std::exception& ex)
        {
            test(_count++ == 0); // Ensure it's only called once
            test(dynamic_cast<const Test::RetryException*>(&ex) != 0);
            return false;
        }

        virtual bool exception()
        {
            //
            // Unexpected
            //
            test(false);
            return false;
        }

    private:

        int _count;
    };
#endif

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
#ifdef ICE_CPP11_MAPPING
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
#else
            _lastStatus =  _servant->ice_dispatch(request, new CallbackI());
#endif
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

#ifdef ICE_CPP11_MAPPING
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
#else
    _lastStatus = _servant->ice_dispatch(request, _defaultCb);
#endif

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
    ICE_SET_EXCEPTION_FROM_CLONE(_exception, e.ice_clone());
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

#ifndef ICE_CPP11_MAPPING
DispatchInterceptorAsyncCallbackI::DispatchInterceptorAsyncCallbackI(AMDInterceptorI& interceptor) :
    _interceptor(interceptor)
{
}

bool
DispatchInterceptorAsyncCallbackI::response()
{
    return true;
}

bool
DispatchInterceptorAsyncCallbackI::exception(const std::exception& ex)
{
    //
    // Only Ice exceptions are raised by this test
    //
    const IceUtil::Exception& ue = dynamic_cast<const IceUtil::Exception&>(ex);
     _interceptor.setException(ue);
    return true;

}

bool
DispatchInterceptorAsyncCallbackI::exception()
{
    //
    // Unexpected
    //
    test(false);
    return true;
}
#endif
