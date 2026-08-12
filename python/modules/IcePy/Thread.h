// Copyright (c) ZeroC, Inc.

#ifndef ICEPY_THREAD_H
#define ICEPY_THREAD_H

#include "Config.h"
#include "Ice/Ice.h"
#include "Util.h"

#include <functional>
#include <future>
#include <memory>
#include <thread>

namespace IcePy
{
    /// Executes a blocking wait in a new detached thread, and returns a future that completes when this wait
    /// completes.
    ///
    /// The wait must not use any Python state: it keeps running - and the returned future remains safe to destroy -
    /// after the Python object that started this wait is deallocated. This is why we don't use std::async here: the
    /// destructor of a future returned by std::async waits for the completion of the task.
    ///
    /// The thread destroys the wait callable before making the future ready. This allows the caller to control when
    /// destructors run.
    ///
    /// Throws when the thread cannot be started, in which case no wait is in progress.
    std::future<void> waitInThread(std::function<void()> wait);

    /// Release Python's Global Interpreter Lock during potentially time-consuming (and non-Python related) work.
    class AllowThreads
    {
    public:
        AllowThreads();
        ~AllowThreads();

    private:
        PyThreadState* _state;
    };

    /// Ensure that the current thread is capable of calling into Python.
    class AdoptThread
    {
    public:
        AdoptThread();
        ~AdoptThread();

    private:
        PyGILState_STATE _state;
    };

    /// ThreadHook ensures that every Ice thread is ready to invoke the Python API.
    /// It also acts as a wrapper for thread notification callbacks.
    class ThreadHook final
    {
    public:
        ThreadHook(PyObject*, PyObject*);

        void start();
        void stop();

    private:
        PyObjectHandle _threadStart;
        PyObjectHandle _threadStop;
    };
}

#endif
