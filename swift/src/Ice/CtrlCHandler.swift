// Copyright (c) ZeroC, Inc.

import Foundation
import IceImpl

public final class CtrlCHandler {
    private let handle = ICECtrlCHandler()

    public func receiveSignal() async -> Int32 {
        await withCheckedContinuation { continuation in
            self.handle.receiveSignal { signal in
                continuation.resume(returning: signal)
            }
        }
    }
}
