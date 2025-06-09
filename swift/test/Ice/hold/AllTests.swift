// Copyright (c) ZeroC, Inc.

import Combine
import Foundation
import Ice
import Synchronization
import TestCommon

class Condition {
    let _value: Mutex<Bool>

    init(_ value: Bool) {
        _value = Mutex(value)
    }

    func set(_ value: Bool) {
        _value.withLock {
            $0 = value
        }
    }

    func value() -> Bool {
        _value.withLock { $0 }
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
    let hold = try await checkedCast(prx: base, type: HoldPrx.self)!
    let holdOneway = uncheckedCast(prx: base.ice_oneway(), type: HoldPrx.self)
    try test(hold == base)
    let holdSerialized = try await checkedCast(prx: baseSerialized, type: HoldPrx.self)!
    let holdSerializedOneway = uncheckedCast(prx: baseSerialized.ice_oneway(), type: HoldPrx.self)
    try test(holdSerialized == baseSerialized)
    output.writeLine("ok")

    output.write("changing state between active and hold rapidly... ")
    for _ in 0..<100 {
        try await hold.putOnHold(0)
    }

    for _ in 0..<100 {
        try await holdOneway.putOnHold(0)
    }

    for _ in 0..<100 {
        try await holdSerialized.putOnHold(0)
    }

    for _ in 0..<100 {
        try await holdSerializedOneway.putOnHold(0)
    }
    output.writeLine("ok")

    output.write("testing waitForHold... ")
    do {
        try await hold.waitForHold()
        try await hold.waitForHold()
        for i in 0..<1000 {
            try await holdOneway.ice_ping()
            if (i % 20) == 0 {
                try await hold.putOnHold(0)
            }
        }
        try await hold.putOnHold(-1)
        try await hold.ice_ping()
        try await hold.putOnHold(-1)
        try await hold.ice_ping()
    }
    output.writeLine("ok")

    output.write("changing state to hold and shutting down server... ")
    try await hold.shutdown()
    output.writeLine("ok")
}
