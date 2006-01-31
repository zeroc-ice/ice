// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_THREAD_H
#define ICE_UTIL_THREAD_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <IceUtil/Mutex.h>

namespace IceUtil
{

class Time;

class ICE_UTIL_API ThreadControl
{
public:

    ThreadControl();

#ifdef _WIN32
    ThreadControl(HANDLE, DWORD);
#else
    ThreadControl(pthread_t);
#endif

    ThreadControl(const ThreadControl&);

    ~ThreadControl();

    ThreadControl& operator=(const ThreadControl&);

    bool operator==(const ThreadControl&) const;
    bool operator!=(const ThreadControl&) const;

    //
    // Wait until the controlled thread terminates. The call has POSIX
    // semantics.
    //
    // At most one thread can wait for the termination of a given
    // thread. Calling join on a thread on which another thread is
    // already waiting for termination results in undefined behaviour,
    // as does joining with a thread after having joined with it
    // previously, or joining with a detached thread.
    //
    void join();

    //
    // Detach a thread. Once a thread is detached, it cannot be
    // detached again, nor can it be joined with. Every thread that
    // was created using the IceUtil::Thread class must either be
    // joined with or detached exactly once. Detaching a thread a
    // second time, or detaching a thread that was previously joined
    // with results in undefined behavior.
    //
    void detach();

    static void sleep(const Time&);
    static void yield();

private:

#ifdef _WIN32
    HANDLE _handle;
    DWORD  _id;
#else
    pthread_t _thread;
#endif
};

class ICE_UTIL_API Thread : virtual public IceUtil::Shared
{
public:

    Thread();
    virtual ~Thread();

    virtual void run() = 0;

    ThreadControl start(size_t = 0);

    ThreadControl getThreadControl() const;

    bool operator==(const Thread&) const;
    bool operator!=(const Thread&) const;
    bool operator<(const Thread&) const;

    //
    // Check whether a thread is still alive. This is useful to implement
    // a non-blocking join().
    //
    bool isAlive() const;

    //
    // This function is an implementation detail;
    // do not call it.
    //
    void done();

protected:
    Mutex _stateMutex;
    bool _started;
    bool _running;

#ifdef _WIN32
    HANDLE _handle;
    DWORD  _id;
#else
    pthread_t _thread;
#endif

private:
    Thread(const Thread&);		// Copying is forbidden
    void operator=(const Thread&);	// Assignment is forbidden
};

typedef Handle<Thread> ThreadPtr;
    
}

#endif

