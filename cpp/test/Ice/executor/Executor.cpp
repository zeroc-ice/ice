// Copyright (c) ZeroC, Inc.

#include "Executor.h"
#include "TestHelper.h"

using namespace std;

shared_ptr<Executor> instance;
std::thread executorThread;

shared_ptr<Executor>
Executor::create()
{
    auto executor = shared_ptr<Executor>(new Executor());
    executorThread = std::thread([executor] { executor->run(); });
    instance = executor;
    return executor;
}

Executor::Executor() { _terminated = false; }

void
Executor::terminate()
{
    std::thread thread;
    {
        lock_guard lock(_mutex);
        _terminated = true;
        _conditionVariable.notify_one();
        thread = std::move(executorThread);
    }

    assert(thread.joinable());
    thread.join();
    instance = nullptr;
}

bool
Executor::isExecutorThread()
{
    return this_thread::get_id() == executorThread.get_id();
}

void
Executor::execute(const shared_ptr<ExecutorCall>& call, const shared_ptr<Ice::Connection>&)
{
    lock_guard lock(_mutex);
    _calls.push_back(call);
    if (_calls.size() == 1)
    {
        _conditionVariable.notify_one();
    }
}

void
Executor::run()
{
    while (true)
    {
        shared_ptr<ExecutorCall> call;
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
