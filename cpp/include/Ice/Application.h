// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_APPLICATION_H
#define ICE_APPLICATION_H

#include <Ice/Ice.h>

#ifndef _WIN32
#   include <csignal>
#endif

namespace Ice
{

#ifdef _WIN32
BOOL WINAPI interruptHandler(DWORD);
#else
void interruptHandler(int);
#endif

class ICE_API Application : public IceUtil::noncopyable
{
public:

    Application();
    virtual ~Application();

    //
    // This main() must be called by the global main(). main()
    // initializes the Communicator, calls run() as a template method,
    // and destroys the Communicator upon return from run(). It
    // thereby handles all exceptions properly, i.e., error messages
    // are printed if exceptions propagate to main(), and the
    // Communicator is always destroyed, regardless of exceptions.
    //
    int main(int, char*[], const char* = 0);
    virtual int run(int, char*[]) = 0;

    //
    // Return the application name, i.e., argv[0].
    //
    static const char* appName();

    //
    // One limitation of this class is that there can only be one
    // Application instance, with one global Communicator, accessible
    // with this communicator() operation. This limitiation is due to
    // how the signal handling functions below operate. If you require
    // multiple Communicators, then you cannot use this Application
    // framework class.
    //
    static CommunicatorPtr communicator();

    //
    // These methods can be used to set a signal handler that calls
    // communicator()->shutdown() upon interrupt (to make
    // communicator()->waitForShutdown() return), to ignore
    // interrupts, or to set interrupts back to default behavior.
    //
    static void shutdownOnInterrupt();
    static void ignoreInterrupt();
    static void defaultInterrupt();

    //
    // These methods can be used to temporarily block a signal and
    // arrange for delivery of a pending signal later. Any signal that
    // is received after holdInterrupt() was called is remember and
    // delivered when releaseInterupt() is called. That signal is then
    // handled according to the signal disposition established with
    // shutdownOnInterrupt(), ignoreInterrupt(), or
    // defaultInterrupt().
    //
    static void holdInterrupt();
    static void releaseInterrupt();

    //
    // This method returns true if a signal handler was triggered,
    // false otherwise. This can be used once
    // Communicator::waitForShutdown() returns to test whether the
    // shutdown was due to an interrupt (interrupted() returns true in
    // that case) or because Communicator::shutdown() was called
    // (interupted() returns false in that case).
    //
    static bool interrupted();

private:

    static const char* _appName;
    static CommunicatorPtr _communicator;
    static bool _interrupted;

#ifdef _WIN32
    friend BOOL WINAPI interruptHandler(DWORD);
#else
    friend void interruptHandler(int);
    static const int signals[];
    static sigset_t signalSet;
#endif
};

};

#endif
