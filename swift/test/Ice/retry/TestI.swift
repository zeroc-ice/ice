//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice

class RetryI: Retry {
    var _counter: Int32

    init() {
        _counter = 0
    }

    func op(kill: Bool, current: Ice.Current) throws {
        if kill {
            if let con = current.con {
                try con.close(.Forcefully)
            } else {
                throw Ice.ConnectionLostException(error: 1)
            }
        }
    }

    func opIdempotent(c: Int32, current: Ice.Current) throws -> Int32 {
        if c > _counter {
            _counter += 1
            throw Ice.ConnectionLostException(error: 1)
        }
        let counter = _counter
        _counter = 0
        return counter
    }

    func opNotIdempotent(current: Ice.Current) throws {
        throw Ice.ConnectionLostException(error: 1)
    }

    func opSystemException(current: Ice.Current) throws {
        throw Ice.RuntimeError("")
    }

    func shutdown(current: Ice.Current) {
        current.adapter!.getCommunicator().shutdown()
    }
}
