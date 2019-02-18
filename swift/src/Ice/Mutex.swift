// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Foundation

struct Mutex {
    var unfairLock = os_unfair_lock()

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
