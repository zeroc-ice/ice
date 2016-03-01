// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_APPLICATION_H
#define ICE_APPLICATION_H

#include <Ice/Ice.h>

#include <IceUtil/Mutex.h>

namespace Ice
{
    enum SignalPolicy { HandleSignals, NoSignalHandling } ;
    class Application;
}

namespace IceInternal
{
    
namespace Application
{
    extern ICE_API IceUtil::Mutex* mutex;
    extern ICE_API IceUtil::Cond* _condVar;

    //
    // Variables than can change while run() and communicator->destroy() are running!
    //
    extern ICE_API bool _callbackInProgress;
    extern ICE_API bool _destroyed;
    extern ICE_API bool _interrupted;
    
    //
    // Variables that are immutable during run() and until communicator->destroy() has returned;
    // before and after run(), and once communicator->destroy() has returned, we assume that 
    // only the main thread and CtrlCHandler threads are running.
    //
    extern ICE_API std::string _appName;
    extern ICE_API Ice::CommunicatorPtr _communicator;
    extern ICE_API Ice::SignalPolicy _signalPolicy;
    extern ICE_API Ice::Application* _application;
}

}

namespace Ice
{

class ICE_API Application : private IceUtil::noncopyable
{
public:

    Application(SignalPolicy = HandleSignals);
    virtual ~Application();

    //
    // This main() must be called by the global main(). main()
    // initializes the Communicator, calls run() as a template method,
    // and destroys the Communicator upon return from run(). It
    // thereby handles all exceptions properly, i.e., error messages
    // are printed if exceptions propagate to main(), and the
    // Communicator is always destroyed, regardless of exceptions.
    //
    int main(int, char*[], const Ice::InitializationData& = Ice::InitializationData());
    int main(int, char*[], const char*);

    int main(int, char* const [], const Ice::InitializationData& = Ice::InitializationData());
    int main(int, char* const [], const char*);

#ifdef _WIN32

    int main(int, wchar_t*[], const Ice::InitializationData& = Ice::InitializationData());
    int main(int, wchar_t*[], const char*);

#endif

    int main(const StringSeq&, const Ice::InitializationData& = Ice::InitializationData());
    int main(const StringSeq&, const char*);

    virtual int run(int, char*[]) = 0;

    //
    // Override this to provide a custom application interrupt
    // hook. You must call callbackOnInterrupt for this method to
    // be called. Note that the interruptCallback can be called
    // concurrently with any other thread (including main) in your
    // application and thus must take appropriate concurrency
    // precautions.
    //
    virtual void interruptCallback(int);

    //
    // Return the application name, i.e., argv[0].
    //
    static const char* appName();

    //
    // One limitation of this class is that there can only be one
    // Application instance, with one global Communicator, accessible
    // with this communicator() operation. This limitation is due to
    // how the signal handling functions below operate. If you require
    // multiple Communicators, then you cannot use this Application
    // framework class.
    //
    static CommunicatorPtr communicator();

    //
    // These methods can be used to set a Ctrl+C Handler callback.
    //
    static void destroyOnInterrupt();
    static void shutdownOnInterrupt();
    static void ignoreInterrupt();
    static void callbackOnInterrupt();

    //
    // These methods can be used to temporarily block a signal and
    // arrange for delivery of a pending signal later. Any signal that
    // is received after holdInterrupt() was called is remembered and
    // delivered when releaseInterupt() is called. That signal is then
    // handled according to the signal disposition established with
    // destroyOnInterrupt(), shutdownOnInterrupt() or
    // ignoreInterrupt().
    //
    static void holdInterrupt();
    static void releaseInterrupt();

    //
    // This method returns true if a signal handler was triggered,
    // false otherwise. This can be used once
    // Communicator::waitForShutdown() returns to test whether the
    // shutdown was due to an interrupt (interrupted() returns true in
    // that case) or because Communicator::shutdown() was called
    // (interrupted() returns false in that case).
    //
    static bool interrupted();

protected:

    virtual int doMain(int, char*[], const Ice::InitializationData&);

#if defined(__SUNPRO_CC)
//
// Sun C++ 5.x does not like classes with no data members 
//
    char _dummy;
#endif
};

}

#endif
