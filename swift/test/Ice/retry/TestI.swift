// Copyright (c) ZeroC, Inc.

import Foundation
import Ice

actor RetryI: Retry {
    var _counter: Int32

    init() {
        _counter = 0
    }

    func op(kill: Bool, current: Ice.Current) async throws {
        if kill {
            if let con = current.con {
                con.abort()
            } else {
                throw Ice.ConnectionLostException("op failed")
            }
        }
    }

    func opIdempotent(c: Int32, current _: Ice.Current) async throws -> Int32 {
        if c < 0 {
            _counter = 0
            return 0
        }
        if c > _counter {
            _counter += 1
            throw Ice.ConnectionLostException("opIdempotent failed")
        }
        let counter = _counter
        _counter = 0
        return counter
    }

    func opNotIdempotent(current _: Ice.Current) async throws {
        throw Ice.ConnectionLostException("opNotIdempotent failed")
    }

    func shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown()
    }
}
