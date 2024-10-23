//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "CallbackExecutor.h"

using namespace std;
using namespace DataStormI;

CallbackExecutor::CallbackExecutor(function<void(function<void()> call)> callbackExecutor)
    : _flush(false),
      _destroyed(false),
      _callbackExecutor(std::move(callbackExecutor))
{
    _thread = thread(
        [this]
        {
            std::vector<function<void()>> queue;
            while (true)
            {
                unique_lock<mutex> lock(_mutex);
                if (_destroyed)
                {
                    break;
                }
                _cond.wait(lock, [this] { return _flush || _destroyed; });
                if (_flush)
                {
                    _flush = false;
                    _queue.swap(queue);
                }

                lock.unlock();
                for (const auto& callback : queue)
                {
                    try
                    {
                        if (_callbackExecutor)
                        {
                            // TODO do we need to ensure that the callback is executed before we continue?
                            _callbackExecutor(callback);
                        }
                        else
                        {
                            callback();
                        }
                    }
                    catch (...)
                    {
                        std::terminate();
                    }
                }
                queue.clear();
            }
        });
}

void
CallbackExecutor::queue(function<void()> cb, bool flush)
{
    unique_lock<mutex> lock(_mutex);
    _queue.emplace_back(std::move(cb));
    if (flush)
    {
        _flush = true;
        _cond.notify_one();
    }
}

void
CallbackExecutor::flush()
{
    unique_lock<mutex> lock(_mutex);
    if (!_queue.empty())
    {
        _flush = true;
        _cond.notify_one();
    }
}

void
CallbackExecutor::destroy()
{
    unique_lock<mutex> lock(_mutex);
    _destroyed = true;
    _cond.notify_one();
    lock.unlock();
    _thread.join();
}
