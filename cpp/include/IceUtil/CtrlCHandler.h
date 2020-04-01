//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_CTRL_C_HANDLER_H
#define ICE_UTIL_CTRL_C_HANDLER_H

#include <IceUtil/Config.h>
#include <IceUtil/Exception.h>

namespace IceUtil
{

/**
 * Invoked when a signal occurs. The callback must not raise exceptions.
 * On Linux and macOS, the callback is NOT a signal handler and can call
 * functions that are not async-signal safe.
 * @param sig The signal number that occurred.
 */
using CtrlCHandlerCallback = std::function<void(int sig)>;

/**
 * Provides a portable way to handle Ctrl-C and Ctrl-C like signals.
 * On Linux and macOS, the CtrlCHandler handles SIGHUP, SIGINT and SIGTERM.
 * On Windows, it is essentially a wrapper for SetConsoleCtrlHandler().
 *
 * \headerfile Ice/Ice.h
 */
class ICE_API CtrlCHandler
{
public:

     /**
     * Registers a callback function that handles Ctrl-C like signals.
     * On Linux and macOS, this constructor masks the SIGHUP, SIGINT and SIGTERM
     * signals and then creates a thread that waits for these signals using sigwait.
     * On Windows, this constructor calls SetConsoleCtrlCHandler to register a handler
     * routine that calls the supplied callback function.
     * Only a single CtrlCHandler object can exist in a process at a give time.
     * @param cb The callback function to invoke when a signal is received.
     */
    explicit CtrlCHandler(CtrlCHandlerCallback cb = nullptr);

     /**
     * Unregisters the callback function.
     * On Linux and macOS, this destructor joins and terminates the thread created
     * by the constructor but does not "unmask" SIGHUP, SIGINT and SIGTERM. As a result,
     * these signals are ignored after this destructor completes.
     * On Windows, this destructor unregisters the SetConsoleCtrlHandler handler
     * routine, and as a result a Ctrl-C or similar signal will terminate the application
     * after this destructor completes.
     */
    ~CtrlCHandler();

    /**
     * Replaces the signal callback.
     * @param cb The new callback.
     * @return The old callback, or nil if no callback is currently set.
     */
    CtrlCHandlerCallback setCallback(CtrlCHandlerCallback cb);

    /**
     * Obtains the signal callback.
     * @return The callback.
     */
    CtrlCHandlerCallback getCallback() const;
};

/**
 * Raised by the CtrlCHandler constructor if another CtrlCHandler already exists.
 *
 * \headerfile Ice/Ice.h
 */
class ICE_API CtrlCHandlerException : public ExceptionHelper<CtrlCHandlerException>
{
public:

    CtrlCHandlerException(const char*, int);
    virtual std::string ice_id() const;

};

}

#endif
