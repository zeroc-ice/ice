// Copyright (c) ZeroC, Inc.

import Combine
import Foundation
import Ice
import TestCommon

class Condition {
    var _lock = os_unfair_lock()
    var _value: Bool

    init(_ value: Bool) {
        _value = value
    }

    func set(_ value: Bool) {
        withLock(&_lock) {
            self._value = value
        }
    }

    func value() -> Bool {
        return withLock(&_lock) {
            self._value
        }
    }
}

func allTests(_ helper: TestHelper) async throws {
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
        let cond = Condition(true)
        var value: Int32 = 0

        var sentTask: Task<Bool, Never>!

        while cond.value() {
            let expected = value
            // The continuation will not always be resumed in the case of a local exception before sending the response.
            // Therefor we use withUnsafeContinuation.
            sentTask = Task {
                await withUnsafeContinuation { continuation in
                    Task {
                        let v = try await hold.setAsync(value: expected + 1, delay: Int32.random(in: 0..<5)) {
                            continuation.resume(returning: $0)
                        }
                        if v != expected {
                            cond.set(false)
                        }
                        return v
                    }
                }
            }

            value += 1
            if value % 100 == 0 {
                _ = await sentTask.value
            }

            if value > 100_000 {
                // Don't continue, it's possible that out-of-order dispatch doesn't occur
                // after 100000 iterations and we don't want the test to last for too long
                // when this occurs.
                break
            }
        }
        try test(value > 100_000 || !cond.value())
        _ = await sentTask.value
    }
    output.writeLine("ok")

    // TODO: Update this test
    // output.write("testing with serialize mode... ")
    // do {
    //     let cond = Condition(true)
    //     var value: Int32 = 0

    //     let completedTask = TestTask<Int32>()

    //     while value < 3000, cond.value() {
    //         let expected = value
    //         let sentTask = Task {
    //             await withUnsafeContinuation { continuation in
    //                 let task = Task {
    //                     let v = try await holdSerialized.setAsync(value: expected + 1, delay: 0) {
    //                         continuation.resume(returning: $0)
    //                     }
    //                     if v != expected {
    //                         cond.set(false)
    //                     }
    //                     return v
    //                 }
    //                 await completedTask.set(task)
    //             }
    //         }

    //         value += 1
    //         if value % 100 == 0 {
    //             _ = await sentTask.value
    //         }
    //     }
    //     _ = try await completedTask.task().value
    //     try test(cond.value())

    //     for i in 0..<10000 {
    //         try holdSerializedOneway.setOneway(value: value + 1, expected: value)
    //         value += 1
    //         if i % 100 == 0 {
    //             try holdSerializedOneway.putOnHold(1)
    //         }
    //     }
    // }
    // output.writeLine("ok")

    // output.write("testing serialization... ")
    // do {
    //     var value: Int32 = 0
    //     _ = try holdSerialized.set(value: value, delay: 0)
    //     // We use the same proxy for all oneway calls.
    //     let holdSerializedOneway = holdSerialized.ice_oneway()
    //     var completed: Task<Void, any Error>!
    //     for i in 0..<10000 {
    //         completed = Task { [value] in
    //             try await holdSerializedOneway.setOnewayAsync(value: value + 1, expected: value)
    //         }
    //         value += 1
    //         if (i % 100) == 0 {
    //             try await completed.value
    //             try holdSerialized.ice_ping()  // Ensure everything's dispatched.
    //             try holdSerialized.ice_getConnection()!.close(.GracefullyWithWait)
    //         }
    //     }
    //     try await completed.value
    // }
    // output.writeLine("ok")

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
