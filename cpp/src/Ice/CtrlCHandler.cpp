// Copyright (c) ZeroC, Inc.

#include "Ice/CtrlCHandler.h"
#include "Ice/LocalException.h"

#ifdef _WIN32
#    include <windows.h>
#else
#    include <csignal>
#endif

#include <cassert>
#include <mutex>

using namespace Ice;
using namespace std;

namespace
{
    CtrlCHandlerCallback _callback = nullptr;

    const CtrlCHandler* _handler = nullptr;

    mutex globalMutex;
}

CtrlCHandler::CtrlCHandler() : CtrlCHandler(nullptr) {}

CtrlCHandlerCallback
CtrlCHandler::setCallback(CtrlCHandlerCallback callback)
{
    lock_guard lock(globalMutex);
    CtrlCHandlerCallback oldCallback = std::move(_callback);
    _callback = std::move(callback);
    return oldCallback;
}

CtrlCHandlerCallback
CtrlCHandler::getCallback() const
{
    lock_guard lock(globalMutex);
    return _callback;
}

#ifdef _WIN32

static BOOL WINAPI
handlerRoutine(DWORD dwCtrlType)
{
    CtrlCHandlerCallback callback;
    {
        lock_guard lock(globalMutex);
        if (!_handler) // The handler is destroyed.
        {
            return FALSE;
        }
        callback = _callback;
    }
    if (callback)
    {
        callback(dwCtrlType);
    }
    return TRUE;
}

CtrlCHandler::CtrlCHandler(CtrlCHandlerCallback callback)
{
    unique_lock lock(globalMutex);
    if (_handler)
    {
        throw Ice::LocalException{__FILE__, __LINE__, "another CtrlCHandler was already created"};
    }
    else
    {
        _callback = std::move(callback);
        _handler = this;
        lock.unlock();

        SetConsoleCtrlHandler(handlerRoutine, TRUE);
    }
}

CtrlCHandler::~CtrlCHandler()
{
    SetConsoleCtrlHandler(handlerRoutine, FALSE);
    lock_guard lock(globalMutex);
    _handler = nullptr;
    _callback = nullptr;
}

#else

extern "C"
{
    static void* sigwaitThread(void*)
    {
        sigset_t ctrlCLikeSignals;
        sigemptyset(&ctrlCLikeSignals);
        sigaddset(&ctrlCLikeSignals, SIGHUP);
        sigaddset(&ctrlCLikeSignals, SIGINT);
        sigaddset(&ctrlCLikeSignals, SIGTERM);

        // Run until the handler is destroyed (!_handler)
        for (;;)
        {
            int signal = 0;
            int rc = sigwait(&ctrlCLikeSignals, &signal);
            if (rc == EINTR)
            {
                // Some sigwait() implementations incorrectly return EINTR when interrupted by an unblocked caught
                // signal.
                continue;
            }
            assert(rc == 0);

            CtrlCHandlerCallback callback;
            {
                lock_guard lock(globalMutex);
                if (!_handler) // The handler is destroyed.
                {
                    return nullptr;
                }
                callback = _callback;
            }

            if (callback)
            {
                callback(signal);
            }
        }
        return nullptr;
    }
}

namespace
{
    pthread_t _tid;
}

CtrlCHandler::CtrlCHandler(CtrlCHandlerCallback callback)
{
    unique_lock lock(globalMutex);
    if (_handler)
    {
        throw Ice::LocalException{__FILE__, __LINE__, "another CtrlCHandler was already created"};
    }
    else
    {
        _callback = std::move(callback);
        _handler = this;

        lock.unlock();

        // We block these CTRL+C like signals in the main thread, and by default all other threads will inherit this
        // signal mask.

        sigset_t ctrlCLikeSignals;
        sigemptyset(&ctrlCLikeSignals);
        sigaddset(&ctrlCLikeSignals, SIGHUP);
        sigaddset(&ctrlCLikeSignals, SIGINT);
        sigaddset(&ctrlCLikeSignals, SIGTERM);

        [[maybe_unused]] int rc = pthread_sigmask(SIG_BLOCK, &ctrlCLikeSignals, nullptr);
        assert(rc == 0);

        // Joinable thread
        rc = pthread_create(&_tid, nullptr, sigwaitThread, nullptr);
        assert(rc == 0);
    }
}

CtrlCHandler::~CtrlCHandler()
{
    // Clear the handler, the sigwaitThread will exit if _handler is null
    {
        lock_guard lock(globalMutex);
        _handler = nullptr;
        _callback = nullptr;
    }

    // Signal the sigwaitThread and join it.
    void* status = nullptr;
    [[maybe_unused]] int rc = pthread_kill(_tid, SIGTERM); // NOLINT(cert-pos44-c)
    assert(rc == 0);
    rc = pthread_join(_tid, &status);
    assert(rc == 0);
}

#endif
