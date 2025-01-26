// Copyright (c) ZeroC, Inc.

#ifndef ICE_CTRL_C_HANDLER_H
#define ICE_CTRL_C_HANDLER_H

#include "Config.h"

#include <functional>

namespace Ice
{
    /// The function called by CtrlCHandler when it catches a signal. This function must not throw exceptions. On Linux
    /// and macOS, this function is NOT a signal handler and can call functions that are not async-signal safe.
    /// @param sig The signal number that occurred.
    /// \headerfile Ice/Ice.h
    using CtrlCHandlerCallback = std::function<void(int sig)>;

    /// Provides a portable way to handle Ctrl-C and Ctrl-C like signals. On Linux and macOS, the CtrlCHandler handles
    /// SIGHUP, SIGINT and SIGTERM. On Windows, it is essentially a wrapper for SetConsoleCtrlHandler.
    /// \headerfile Ice/Ice.h
    class ICE_API CtrlCHandler
    {
    public:
        /// Constructs a CtrlCHandler.
        /// On Linux and macOS, this constructor masks the SIGHUP, SIGINT and SIGTERM signals and then creates a thread
        /// that waits for these signals using sigwait. On Windows, this constructor calls SetConsoleCtrlCHandler to
        /// register a handler routine that calls the supplied callback function.
        /// Only a single CtrlCHandler object can exist in a process at a give time.
        /// @param cb The callback function to invoke when a signal is received. The default (nullptr) means do nothing.
        explicit CtrlCHandler(CtrlCHandlerCallback cb = nullptr);

        /// Unregisters the callback function.
        /// On Linux and macOS, this destructor joins and terminates the thread created by the constructor but does not
        /// "unmask" SIGHUP, SIGINT and SIGTERM. As a result, these signals are ignored after this destructor completes.
        /// On Windows, this destructor unregisters the SetConsoleCtrlHandler handler routine, and as a result a
        /// Ctrl-C or similar signal will terminate the application after this destructor completes.
        ~CtrlCHandler();

        /// Replaces the signal callback.
        /// @param cb The new callback.
        /// @return The old callback, which may be nullptr.
        CtrlCHandlerCallback setCallback(CtrlCHandlerCallback cb);

        /// Obtains the current signal callback.
        /// @return The callback.
        [[nodiscard]] CtrlCHandlerCallback getCallback() const;
    };

    /// Waits until the CtrlC-C handler catches a signal.
    /// @param handler The Ctrl-C handler.
    /// @return The signal number that was caught.
    /// @remark This function installs a callback in the CtrlCHandler object. It must not be called when a non-null
    /// callback is already installed.
    ICE_API int wait(CtrlCHandler& handler);
}

#endif
