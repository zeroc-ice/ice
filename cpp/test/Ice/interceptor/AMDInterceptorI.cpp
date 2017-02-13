// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <AMDInterceptorI.h>
#include <Test.h>
#include <TestCommon.h>

AMDInterceptorI::AMDInterceptorI(const Ice::ObjectPtr& servant) :
    InterceptorI(servant),
    _defaultCb(new DispatchInterceptorAsyncCallbackI(*this)),
    _actualStatus(Ice::DispatchAsync)
   
{
}

    
Ice::DispatchStatus 
AMDInterceptorI::dispatch(Ice::Request& request)
{
    class CallbackI : public Ice::DispatchInterceptorAsyncCallback
    {
    public:
        
        virtual bool response(bool ok)
        {
            test(ok);
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


    Ice::Current& current = const_cast<Ice::Current&>(request.getCurrent());
    _lastOperation = current.operation;
    
    Ice::DispatchInterceptorAsyncCallbackPtr cb = new CallbackI();

    if(_lastOperation == "amdAddWithRetry")
    {
        for(int i = 0; i < 10; ++i)
        {
            _lastStatus =  _servant->ice_dispatch(request, cb);
            test(_lastStatus == Ice::DispatchAsync);
        }
        
        current.ctx["retry"] = "no";
    }
      
    _lastStatus = _servant->ice_dispatch(request, _defaultCb);
    return _lastStatus;
}

void 
AMDInterceptorI::setActualStatus(Ice::DispatchStatus status)
{
    IceUtil::Mutex::Lock lock(_mutex);
    _actualStatus = status;
}

void 
AMDInterceptorI::setActualStatus(const IceUtil::Exception& e)
{
    IceUtil::Mutex::Lock lock(_mutex);
    _exception.reset(e.ice_clone());
    _actualStatus = Ice::DispatchAsync;
}

Ice::DispatchStatus
AMDInterceptorI::getActualStatus() const
{
    IceUtil::Mutex::Lock lock(_mutex);
    return _actualStatus;
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
    _actualStatus = Ice::DispatchAsync;
    _exception.reset();
}


DispatchInterceptorAsyncCallbackI::DispatchInterceptorAsyncCallbackI(AMDInterceptorI& interceptor) :
    _interceptor(interceptor)
{
}

bool 
DispatchInterceptorAsyncCallbackI::response(bool ok)
{
    _interceptor.setActualStatus(ok ? Ice::DispatchOK : Ice::DispatchUserException);
    return true;
}


bool 
DispatchInterceptorAsyncCallbackI::exception(const std::exception& ex)
{
    //
    // Only Ice exceptions are raised by this test
    //
    const IceUtil::Exception& ue = dynamic_cast<const IceUtil::Exception&>(ex);
     _interceptor.setActualStatus(ue);
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
