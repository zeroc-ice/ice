// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Dispatcher.h>
#include <TestCommon.h>

using namespace std;

IceUtil::Handle<Dispatcher> Dispatcher::_instance;

Dispatcher::Dispatcher()
{
    _instance = this;
    _terminated = false;
    iceSetNoDelete(true);
    start();
    iceSetNoDelete(false);
}

void
Dispatcher::terminate()
{
    {
        Lock sync(*_instance);
        _instance->_terminated = true;
        _instance->notify();
    }

    _instance->getThreadControl().join();
    _instance = 0;
}

bool
Dispatcher::isDispatcherThread()
{
    return IceUtil::ThreadControl() == _instance->getThreadControl();
}

#ifdef ICE_CPP11_MAPPING
void
Dispatcher::dispatch(const shared_ptr<DispatcherCall>& call, const shared_ptr<Ice::Connection>&)
#else
void
Dispatcher::dispatch(const Ice::DispatcherCallPtr& call, const Ice::ConnectionPtr&)
#endif
{
    Lock sync(*this);
    _calls.push_back(call);
    if(_calls.size() == 1)
    {
        notify();
    }
}

void
Dispatcher::run()
{
    while(true)
    {
#ifdef ICE_CPP11_MAPPING
        shared_ptr<DispatcherCall> call;
#else
        Ice::DispatcherCallPtr call;
#endif
        {
            Lock sync(*this);

            while(!_terminated && _calls.empty())
            {
                wait();
            }

            if(!_calls.empty())
            {
                call = _calls.front();
                _calls.pop_front();
            }
            else if(_terminated)
            {
                // Terminate only once all calls are dispatched.
                return;
            }
        }


        if(call)
        {
            try
            {
                call->run();
            }
            catch(...)
            {
                // Exceptions should never propagate here.
                test(false);
            }
        }
    }
}
