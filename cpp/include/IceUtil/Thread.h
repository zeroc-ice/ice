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

#ifdef _WIN32
struct HandleWrapper : public Shared
{
    // Inline for performance reasons.
    HandleWrapper(HANDLE h) :
	handle(h)
    {
    }

    // Inline for performance reasons.
    virtual ~HandleWrapper()
    {
	if(handle)
	{
	    CloseHandle(handle);
	}
    }

    HANDLE handle;
};

typedef Handle<HandleWrapper> HandleWrapperPtr;
#endif

#ifdef _WIN32
    typedef unsigned int ThreadId;
#else
    typedef pthread_t ThreadId;
#endif

class ICE_UTIL_API ThreadControl
{
public:

    ThreadControl();

#ifdef _WIN32
    ThreadControl(const HandleWrapperPtr&, ThreadId);
#else
    ThreadControl(ThreadId);
#endif

    ThreadControl(const ThreadControl&);
    ThreadControl& operator=(const ThreadControl&);

    bool operator==(const ThreadControl&) const;
    bool operator!=(const ThreadControl&) const;
    bool operator<(const ThreadControl&) const;

    //
    // Return the ID of the thread underlying this ThreadControl.
    //
    ThreadId id() const;

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

    //
    // Check whether a thread is still alive. This is useful to implement
    // a non-blocking join().
    //
    bool isAlive() const;

    static void sleep(const Time&);
    static void yield();

private:

    Mutex _stateMutex;
#ifdef _WIN32
    HandleWrapperPtr _handle;
#endif
    ThreadId _id;
};

class ICE_UTIL_API Thread : virtual public IceUtil::Shared
{
public:

    Thread();
    virtual ~Thread();

    ThreadId id() const;

    virtual void run() = 0;

    ThreadControl start(size_t = 0);

    ThreadControl getThreadControl() const;

    bool operator==(const Thread&) const;
    bool operator!=(const Thread&) const;
    bool operator<(const Thread&) const;

    Thread(const Thread&);		// Copying is forbidden
    void operator=(const Thread&);	// Assignment is forbidden

private:

    Mutex _stateMutex;
    bool _started;
#ifdef _WIN32
    HandleWrapperPtr _handle;
#endif
    ThreadId _id;
};

typedef Handle<Thread> ThreadPtr;
    
}

#endif

