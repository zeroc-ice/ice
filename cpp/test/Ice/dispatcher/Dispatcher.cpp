//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Dispatcher.h>
#include <TestHelper.h>

using namespace std;

shared_ptr<Dispatcher> instance;
std::thread dispatcherThread;

shared_ptr<Dispatcher>
Dispatcher::create()
{
    auto dispatcher = shared_ptr<Dispatcher>(new Dispatcher());
    dispatcherThread = std::thread([dispatcher] { dispatcher->run(); });
    instance = dispatcher;
    return dispatcher;
}

Dispatcher::Dispatcher() { _terminated = false; }

void
Dispatcher::terminate()
{
    std::thread thread;
    {
        lock_guard lock(_mutex);
        _terminated = true;
        _conditionVariable.notify_one();
        thread = std::move(dispatcherThread);
    }

    assert(thread.joinable());
    thread.join();
    instance = nullptr;
}

bool
Dispatcher::isDispatcherThread()
{
    return this_thread::get_id() == dispatcherThread.get_id();
}

void
Dispatcher::dispatch(const shared_ptr<DispatcherCall>& call, const shared_ptr<Ice::Connection>&)
{
    lock_guard lock(_mutex);
    _calls.push_back(call);
    if (_calls.size() == 1)
    {
        _conditionVariable.notify_one();
    }
}

void
Dispatcher::run()
{
    while (true)
    {
        shared_ptr<DispatcherCall> call;
        {
            unique_lock lock(_mutex);
            _conditionVariable.wait(lock, [this] { return _terminated || !_calls.empty(); });

            if (!_calls.empty())
            {
                call = _calls.front();
                _calls.pop_front();
            }
            else if (_terminated)
            {
                // Terminate only once all calls are dispatched.
                return;
            }
        }

        if (call)
        {
            try
            {
                call->run();
            }
            catch (...)
            {
                // Exceptions should never propagate here.
                test(false);
            }
        }
    }
}
