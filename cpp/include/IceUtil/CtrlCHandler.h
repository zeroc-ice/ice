// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

/**
 * Invoked when a signal occurs. The callback must not raise exceptions.
 * On Unix/POSIX, the callback is NOT a signal handler and can call
 * functions that are not async-signal safe.
 * @param sig The signal number that occurred.
 */
#ifdef ICE_CPP11_MAPPING
using CtrlCHandlerCallback = std::function<void(int sig)>;
#else
typedef void (*CtrlCHandlerCallback)(int sig);
#endif

/**
 * Provides a portable way to handle CTRL+C and CTRL+C like signals.
 * On Unix/POSIX, the CtrlCHandler handles SIGHUP, SIGINT and SIGTERM.
 * On Windows, it is essentially a wrapper for SetConsoleCtrlHandler().
 *
 * In a process, only one CtrlCHandler can exist at a given time:
 * the CtrlCHandler constructor raises CtrlCHandlerException if
 * you attempt to create a second CtrlCHandler.
 * On Unix/POSIX, it is essential to create the CtrlCHandler before
 * creating any thread, as the CtrlCHandler constructor masks (blocks)
 * SIGHUP, SIGINT and SIGTERM; by default, threads created later will
 * inherit this signal mask.
 *
 * When a CTRL+C or CTRL+C like signal is sent to the process, the
 * user-registered callback is called in a separate thread; it is
 * given the signal number. The callback must not raise exceptions.
 * On Unix/POSIX, the callback is NOT a signal handler and can call
 * functions that are not async-signal safe.
 *
 * The CtrCHandler destructor "unregisters" the callback. However
 * on Unix/POSIX it does not restore the old signal mask in any
 * thread, so SIGHUP, SIGINT and SIGTERM remain blocked.
 *
 * \headerfile IceUtil/CtrlCHandler.h
 */
//
// TODO: Maybe the behavior on Windows should be the same? Now we
// just restore the default behavior (TerminateProcess).
//
class ICE_API CtrlCHandler
{
public:

    /**
     * Initializes the relevant signals.
     * @param cb The signal callback.
     */
    explicit CtrlCHandler(CtrlCHandlerCallback cb = ICE_NULLPTR);
    ~CtrlCHandler();

    /**
     * Replaces the signal callback.
     * @param cb The new callback.
     * @return The old callback, or nil if no callback is currently set.
     */
    CtrlCHandlerCallback setCallback(CtrlCHandlerCallback cb);

    /**
     * Obtains the signal callback.
     * @return The callback, or nil if no callback is currently set.
     */
    CtrlCHandlerCallback getCallback() const;
};

/**
 * Raised by CtrlCHandler.
 *
 * \headerfile IceUtil/CtrlCHandler.h
 */
class ICE_API CtrlCHandlerException : public ExceptionHelper<CtrlCHandlerException>
{
public:

    CtrlCHandlerException(const char*, int);
    virtual std::string ice_id() const;

#ifndef ICE_CPP11_MAPPING
    virtual CtrlCHandlerException* ice_clone() const;
#endif
};

}

#endif
