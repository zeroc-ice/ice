// Copyright (c) ZeroC, Inc.

import Foundation

/// A low-level mutex with no isolation checks.
struct UncheckedMutex {
    private var unfairLock = os_unfair_lock()

    mutating func sync<R>(_ closure: () throws -> R) rethrows -> R {
        os_unfair_lock_lock(&unfairLock)
        defer { os_unfair_lock_unlock(&unfairLock) }
        return try closure()
    }
}
