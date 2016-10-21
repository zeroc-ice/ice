// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <AMDInterceptorI.h>
#include <Test.h>
#include <TestCommon.h>

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

        virtual bool response()
        {
            return false;
        }

        virtual bool exception(const std::exception& ex)
        {
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
    };
    Ice::DispatchInterceptorAsyncCallbackPtr cb = ICE_MAKE_SHARED(CallbackI);
#endif

    Ice::Current& current = const_cast<Ice::Current&>(request.getCurrent());
    _lastOperation = current.operation;

    if(_lastOperation == "amdAddWithRetry")
    {
        for(int i = 0; i < 10; ++i)
        {
            try
            {
#ifdef ICE_CPP11_MAPPING
                _lastStatus =  _servant->ice_dispatch(request, nullptr, [](exception_ptr ex) {
                    try
                    {
                        rethrow_exception(ex);
                    }
                    catch(Test::RetryException&)
                    {
                    }
                    catch(...)
                    {
                        test(false);
                    }
                    return false;
                });
#else
                _lastStatus =  _servant->ice_dispatch(request, cb);
#endif
                test(_lastStatus);
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

#ifdef ICE_CPP11_MAPPING
    _lastStatus = _servant->ice_dispatch(request, []() { return true; }, [this](exception_ptr ex) {
        try
        {
            rethrow_exception(ex);
        }
        catch(const IceUtil::Exception& ex)
        {
            setException(ex);
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
