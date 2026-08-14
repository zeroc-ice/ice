// Copyright (c) ZeroC, Inc.

#include "Thread.h"

using namespace std;
using namespace IcePy;

std::future<void>
IcePy::waitInThread(std::function<void()> wait)
{
    auto promise{make_shared<std::promise<void>>()};
    std::future<void> future{promise->get_future()};

    std::thread{[wait = std::move(wait), promise]() mutable
                {
                    exception_ptr ex;
                    try
                    {
                        wait();
                    }
                    catch (...)
                    {
                        ex = current_exception();
                    }

                    wait = nullptr; // destroy the captures before making the future ready

                    if (ex)
                    {
                        promise->set_exception(ex);
                    }
                    else
                    {
                        promise->set_value();
                    }
                }}
        .detach();

    return future;
}

IcePy::AllowThreads::AllowThreads() { _state = PyEval_SaveThread(); }

IcePy::AllowThreads::~AllowThreads() { PyEval_RestoreThread(_state); }

IcePy::AdoptThread::AdoptThread() { _state = PyGILState_Ensure(); }

IcePy::AdoptThread::~AdoptThread() { PyGILState_Release(_state); }

IcePy::ThreadHook::ThreadHook(PyObject* threadStart, PyObject* threadStop)
    : _threadStart(Py_XNewRef(threadStart)),
      _threadStop(Py_XNewRef(threadStop))
{
    if (threadStart && !PyCallable_Check(threadStart))
    {
        throw Ice::InitializationException(__FILE__, __LINE__, "threadStart must be a callable");
    }

    if (threadStop && !PyCallable_Check(threadStop))
    {
        throw Ice::InitializationException(__FILE__, __LINE__, "threadStop must be a callable");
    }
}

void
IcePy::ThreadHook::start()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.
    if (_threadStart.get())
    {
        PyObjectHandle args{PyTuple_New(0)};
        PyObjectHandle tmp{PyObject_Call(_threadStart.get(), args.get(), nullptr)};
        if (!tmp.get())
        {
            throwPythonException();
        }
    }
}

void
IcePy::ThreadHook::stop()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.
    if (_threadStop.get())
    {
        PyObjectHandle args{PyTuple_New(0)};
        PyObjectHandle tmp{PyObject_Call(_threadStop.get(), args.get(), nullptr)};
        if (!tmp.get())
        {
            throwPythonException();
        }
    }
}
