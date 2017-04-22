// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_APPLICATION_H
#define ICE_APPLICATION_H

#include <Ice/Initialize.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Cond.h>
#include <IceUtil/CtrlCHandler.h>

namespace Ice
{

using IceUtil::CtrlCHandler;

#ifdef ICE_CPP11_MAPPING
enum class SignalPolicy : unsigned char
#else
enum SignalPolicy
#endif
{ HandleSignals, NoSignalHandling };

class ICE_API Application
{
public:

    Application(SignalPolicy = ICE_ENUM(SignalPolicy, HandleSignals));

#ifdef ICE_CPP11_MAPPING
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
#endif

    virtual ~Application();

    // This main() must be called by the global main(). main()
    // initializes the Communicator, calls run() and destroys the
    // the Communicator upon return from run(). It handles all
    // exceptions properly, i.e., error message are printed if
    // exceptions propagate to main(), and the Communicator is always
    // destroyed, regardless of exceptions.
    //
    int main(int, const char* const[], const InitializationData& = InitializationData(), int = ICE_INT_VERSION);
    int main(int, const char* const[], ICE_CONFIG_FILE_STRING, int = ICE_INT_VERSION);

#ifdef _WIN32
    int main(int, const wchar_t* const[], const InitializationData& = InitializationData(), int = ICE_INT_VERSION);
    int main(int, const wchar_t* const[], ICE_CONFIG_FILE_STRING, int = ICE_INT_VERSION);
#endif

    int main(const StringSeq&, const InitializationData& = InitializationData(), int = ICE_INT_VERSION);
    int main(const StringSeq&, ICE_CONFIG_FILE_STRING, int = ICE_INT_VERSION);

    //
    // run is given a copy of the remaining argc/argv arguments,
    // after the communicator initialization in the caller (main)
    // has removed all Ice-related arguments.
    //
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

    virtual int doMain(int, char*[], const InitializationData&, Int);

    //
    // _mutex and _condVar are used to synchronize the main thread and
    // the CtrlCHandler thread
    //
    static IceUtil::Mutex _mutex;
    static IceUtil::Cond _condVar;

    //
    // Variables than can change while run() and communicator->destroy()
    //  are running!
    //
    static bool _callbackInProgress;
    static bool _destroyed;
    static bool _interrupted;

    //
    // Variables that are immutable during run() and until
    // communicator->destroy() has returned, before and after run(), and
    // once communicator->destroy() has returned, we assume that
    // only the main thread and CtrlCHandler threads are running.
    //
    static std::string _appName;
    static CommunicatorPtr _communicator;
    static SignalPolicy _signalPolicy;
    static Application* _application;

private:

    static void holdInterruptCallback(int);
    static void destroyOnInterruptCallback(int);
    static void shutdownOnInterruptCallback(int);
    static void callbackOnInterruptCallback(int);

#ifndef ICE_CPP11_MAPPING
    //
    // Not defined, make Application non-copyable
    //
    Application(const Application&);
    Application& operator=(const Application&);
#endif

};

}

#endif
