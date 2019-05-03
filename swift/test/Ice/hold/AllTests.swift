//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit
import Foundation

class Condition {

    var _lock = os_unfair_lock()
    var _value: Bool

    init(value: Bool) {
        _value = value
    }

    func set(value: Bool) {
        withLock(&_lock) {
            self._value = value
        }
    }

    func value() -> Bool {
        return withLock(&_lock) {
            return self._value
        }
    }
}

func allTests(_ helper: TestHelper) throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()

    let communicator = helper.communicator()

    output.write("testing stringToProxy... ")
    let base = try communicator.stringToProxy("hold:\(helper.getTestEndpoint(num: 0))")!
    let baseSerialized = try communicator.stringToProxy("hold:\(helper.getTestEndpoint(num: 1))")!
    output.writeLine("ok")

    output.write("testing checked cast... ")
    let hold = try checkedCast(prx: base, type: HoldPrx.self)!
    let holdOneway = uncheckedCast(prx: base.ice_oneway(), type: HoldPrx.self)
    try test(hold == base)
    let holdSerialized = try checkedCast(prx: baseSerialized, type: HoldPrx.self)!
    let holdSerializedOneway = uncheckedCast(prx: baseSerialized.ice_oneway(), type: HoldPrx.self)
    try test(holdSerialized == baseSerialized)
    output.writeLine("ok")

    output.write("changing state between active and hold rapidly... ")
    for _ in 0..<100 {
        try hold.putOnHold(0)
    }

    for _ in 0..<100 {
        try holdOneway.putOnHold(0)
    }

    for _ in 0..<100 {
        try holdSerialized.putOnHold(0)
    }

    for _ in 0..<100 {
        try holdSerializedOneway.putOnHold(0)
    }
    output.writeLine("ok")

    output.write("testing without serialize mode... ")
    do {
        let cond = Condition(value: true)
        var value: Int32 = 0

        var completed: Promise<Int32>!
        var sent: Promise<Bool>!
        while cond.value() {

            let expected = value
            sent = Promise<Bool> { seal in
                completed = hold.setAsync(value: value + 1,
                                          delay: Int32.random(in: 0..<5),
                                          sent: {
                                              seal.fulfill($0)
                                          })
            }

            _ = completed!.done { (v: Int32) throws -> Void in
                if v != expected {
                    cond.set(value: false)
                }
            }

            value += 1
            if value % 100 == 0 {
                _ = try sent.wait()
            }

            if value > 100000 {
                // Don't continue, it's possible that out-of-order dispatch doesn't occur
                // after 100000 iterations and we don't want the test to last for too long
                // when this occurs.
                break
            }
        }
        try test(value > 100000 || !cond.value())
        _ = try sent.wait()
    }
    output.writeLine("ok")

    output.write("testing with serialize mode... ")
    do {
        let cond = Condition(value: true)
        var value: Int32 = 0

        var completed: Promise<Int32>?
        while value < 3000 && cond.value() {
            let expected = value
            let sent = Promise<Bool> { seal in
                completed = holdSerialized.setAsync(value: value + 1, delay: 0,
                                          sent: {
                                              seal.fulfill($0)
                                          })
            }

            _ = completed!.done { (v: Int32) throws -> Void in
                if v != expected {
                    cond.set(value: false)
                }
            }
            value += 1
            if value % 100 == 0 {
                _ = try sent.wait()
            }
        }
        _ = try completed!.wait()
        try test(cond.value())

        for i in 0..<10000 {
            try holdSerializedOneway.setOneway(value: value + 1, expected: value)
            value += 1
            if i % 100 == 0 {
                try holdSerializedOneway.putOnHold(1)
            }
        }
    }
    output.writeLine("ok")

    output.write("testing serialization... ")
    do {
        var value: Int32 = 0
        _ = try holdSerialized.set(value: value, delay: 0)
        var completed: Promise<Void>!
        for i in 0..<10000 {
            // Create a new proxy for each request
            completed = holdSerialized.ice_oneway().setOnewayAsync(value: value + 1, expected: value)
            value += 1
            if (i % 100) == 0 {
                try completed.wait()
                try holdSerialized.ice_ping() // Ensure everything's dispatched.
                try holdSerialized.ice_getConnection()!.close(.GracefullyWithWait)
            }
        }
        try completed.wait()
    }
    output.writeLine("ok")

    output.write("testing waitForHold... ")
    do {
        try hold.waitForHold()
        try hold.waitForHold()
        for i in 0..<1000 {
            try holdOneway.ice_ping()
            if (i % 20) == 0 {
                try hold.putOnHold(0)
            }
        }
        try hold.putOnHold(-1)
        try hold.ice_ping()
        try hold.putOnHold(-1)
        try hold.ice_ping()
    }
    output.writeLine("ok")

    output.write("changing state to hold and shutting down server... ")
    try hold.shutdown()
    output.writeLine("ok")
}
