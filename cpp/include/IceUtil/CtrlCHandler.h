// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_CTRL_C_HANDLER_H
#define ICE_UTIL_CTRL_C_HANDLER_H

#include <IceUtil/Config.h>
#include <IceUtil/Exception.h>

namespace IceUtil
{

// The CtrlCHandler provides a portable way to handle CTRL+C and
// CTRL+C like signals 
// On Unix/POSIX, the CtrlCHandler handles SIGHUP, SIGINT and SIGTERM. 
// On Windows, it is essentially a wrapper for SetConsoleCtrlHandler().
// 
// In a process, only one CtrlCHandler can exist at a given time: 
// the CtrlCHandler constructor raises CtrlCHandlerException if
// you attempt to create a second CtrlCHandler.
// On Unix/POSIX, it is essential to create the CtrlCHandler before
// creating any thread, as the CtrlCHandler constructor masks (blocks)
// SIGHUP, SIGINT and SIGTERM; by default, threads created later will 
// inherit this signal mask.
//
// When a CTRL+C or CTRL+C like signal is sent to the process, the 
// user-registered callback is called in a separate thread; it is 
// given the signal number. The callback must not raise exceptions.
// On Unix/POSIX, the callback is NOT a signal handler and can call 
// functions that are not async-signal safe.
//
// The CtrCHandler destructor "unregisters" the callback. However
// on Unix/POSIX it does not restore the old signal mask in any
// thread, so SIGHUP, SIGINT and SIGTERM remain blocked.
//
// TODO: Maybe the behavior on Windows should be the same? Now we
// just restore the default behavior (TerminateProcess).

typedef void (*CtrlCHandlerCallback)(int);

class ICE_UTIL_API CtrlCHandler
{
public:

    CtrlCHandler(CtrlCHandlerCallback = 0);
    ~CtrlCHandler();

    void setCallback(CtrlCHandlerCallback);
    CtrlCHandlerCallback getCallback() const;
};

class ICE_UTIL_API CtrlCHandlerException : public Exception
{ 
public:
 
    CtrlCHandlerException(const char*, int);
    virtual std::string ice_name() const;
    virtual CtrlCHandlerException* ice_clone() const;
    virtual void ice_throw() const;
};

}

#endif
