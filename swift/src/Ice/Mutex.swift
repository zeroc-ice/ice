//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation

struct Mutex {
    private var unfairLock = os_unfair_lock()

    mutating func lock() {
        os_unfair_lock_lock(&unfairLock)
    }

    mutating func locked() -> Bool {
        return os_unfair_lock_trylock(&unfairLock)
    }

    mutating func unlock() {
        os_unfair_lock_unlock(&unfairLock)
    }

    mutating func sync<R>(_ closure: () throws -> R) rethrows -> R {
        lock()
        defer { unlock() }
        return try closure()
    }
}
