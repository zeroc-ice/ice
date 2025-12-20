// Copyright (c) ZeroC, Inc.

import IceImpl

/// Helps applications handle Ctrl+C (SIGINT) and similar signals (SIGHUP and SIGTERM). Only available on macOS.
@available(macOS 15, *)
public struct CtrlCHandler {
    private let handle = ICECtrlCHandler()

    /// Creates a CtrlCHandler. Only one instance of this struct can exist in a program at any point in time.
    /// This instance must be created before starting any thread.
    public init() {}

    /// Sets the signal callback. If there was a previous callback, it is replaced.
    ///
    /// - Parameter callback: The callback to call when a signal is caught; its parameter is the signal number.
    ///   If `callback` is `nil`, any caught signals are ignored.
    public func setCallback(_ callback: ((Int32) -> Void)?) {
        self.handle.setCallback(callback)
    }
}
