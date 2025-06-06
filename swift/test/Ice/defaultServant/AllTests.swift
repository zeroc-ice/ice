// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

final class MyObjectI: MyObject, Ice.Object {
    func ice_id(current: Ice.Current) -> String {
        return Ice.DefaultObject<MyObjectTraits>().ice_id(current: current)
    }

    func ice_ids(current: Ice.Current) -> [String] {
        return Ice.DefaultObject<MyObjectTraits>().ice_ids(current: current)
    }

    func ice_isA(id: String, current: Ice.Current) -> Bool {
        return Ice.DefaultObject<MyObjectTraits>().ice_isA(id: id, current: current)
    }

    func ice_ping(current: Ice.Current) throws {
        if current.id.name == "ObjectNotExist" {
            throw Ice.ObjectNotExistException()
        } else if current.id.name == "FacetNotExist" {
            throw Ice.FacetNotExistException()
        }
    }

    func getName(current: Ice.Current) throws -> String {
        if current.id.name == "ObjectNotExist" {
            throw Ice.ObjectNotExistException()
        } else if current.id.name == "FacetNotExist" {
            throw Ice.FacetNotExistException()
        }
        return current.id.name
    }
}

func allTests(_ helper: TestHelper) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    let output = helper.getWriter()

    let oa = try communicator.createObjectAdapterWithEndpoints(
        name: "MyOA", endpoints: "tcp -h localhost")
    try oa.activate()

    let servant = MyObjectI()

    //
    // Register default servant with category "foo"
    //
    try oa.addDefaultServant(servant: servant, category: "foo")

    //
    // Start test
    //
    output.write("testing single category... ")

    var r = oa.findDefaultServant("foo")
    try test(r as? MyObjectI === servant)

    r = oa.findDefaultServant("bar")
    try test(r == nil)

    var identity = Ice.Identity()
    identity.category = "foo"

    let names = ["foo", "bar", "x", "y", "abcdefg"]

    var prx: MyObjectPrx!
    for name in names {
        identity.name = name
        prx = try uncheckedCast(prx: oa.createProxy(identity), type: MyObjectPrx.self)
        try await prx.ice_ping()
        try await test(prx.getName() == name)
    }

    identity.name = "ObjectNotExist"
    prx = try uncheckedCast(prx: oa.createProxy(identity), type: MyObjectPrx.self)
    do {
        try await prx.ice_ping()
        try test(false)
    } catch is Ice.ObjectNotExistException {}  // Expected

    do {
        _ = try await prx.getName()
        try test(false)
    } catch is Ice.ObjectNotExistException {}  // Expected

    identity.name = "FacetNotExist"
    prx = try uncheckedCast(prx: oa.createProxy(identity), type: MyObjectPrx.self)
    do {
        try await prx.ice_ping()
        try test(false)
    } catch is Ice.FacetNotExistException {}  // Expected

    do {
        _ = try await prx.getName()
        try test(false)
    } catch is Ice.FacetNotExistException {}  // Expected

    identity.category = "bar"
    for name in names {
        identity.name = name
        prx = try uncheckedCast(prx: oa.createProxy(identity), type: MyObjectPrx.self)

        do {
            try await prx.ice_ping()
            try test(false)
        } catch is Ice.ObjectNotExistException {}  // Expected

        do {
            _ = try await prx.getName()
            try test(false)
        } catch is Ice.ObjectNotExistException {}  // Expected
    }

    _ = try oa.removeDefaultServant("foo")
    identity.category = "foo"
    prx = try uncheckedCast(prx: oa.createProxy(identity), type: MyObjectPrx.self)
    do {
        try await prx.ice_ping()
        try test(false)
    } catch is Ice.ObjectNotExistException {}  // Expected

    output.writeLine("ok")

    output.write("testing default category... ")

    try oa.addDefaultServant(servant: servant, category: "")

    r = oa.findDefaultServant("bar")
    try test(r == nil)

    r = oa.findDefaultServant("")
    try test(r as? MyObjectI === servant)

    for name in names {
        identity.name = name
        prx = try uncheckedCast(prx: oa.createProxy(identity), type: MyObjectPrx.self)
        try await prx.ice_ping()
        try await test(prx.getName() == name)
    }
    output.writeLine("ok")
}
