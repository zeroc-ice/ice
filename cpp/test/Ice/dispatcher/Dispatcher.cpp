//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Dispatcher.h>
#include <TestHelper.h>

using namespace std;

IceUtil::Handle<Dispatcher> Dispatcher::_instance;

Dispatcher::Dispatcher()
{
    _instance = this;
    _terminated = false;
    __setNoDelete(true);
    start();
    __setNoDelete(false);
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

void
Dispatcher::dispatch(const shared_ptr<DispatcherCall>& call, const shared_ptr<Ice::Connection>&)
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
        shared_ptr<DispatcherCall> call;
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
