// Copyright (c) ZeroC, Inc.

import IceImpl

/// Helps applications handle Ctrl+C (SIGINT) and similar signals (SIGHUP and SIGTERM).
public final class CtrlCHandler {
    private let handle = ICECtrlCHandler()

    /// Creates a CtrlCHandler. Only one instance of this class can exist in a program at any point in time.
    /// This instance must be created before starting any thread.
    public init() {}

    /// Waits until a Ctrl+C or similar signal is received by this handler.
    /// - Returns: The signal number received.
    public func receiveSignal() async -> Int32 {
        return await withCheckedContinuation { continuation in
            self.handle.receiveSignal { signal in
                continuation.resume(returning: signal)
            }
        }
    }
}
