// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_APPLICATION_H
#define ICE_APPLICATION_H

#include <Ice/Ice.h>

namespace Ice
{

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

private:

    static const char* _appName;
    static CommunicatorPtr _communicator;
};

};

#endif
