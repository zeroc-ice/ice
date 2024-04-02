//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

final class MyObjectI: ObjectI<MyObjectTraits>, MyObject {
    override func ice_ping(current: Ice.Current) throws {
        if current.id.name == "ObjectNotExist" {
            throw Ice.ObjectNotExistException(id: current.id, facet: "", operation: "ice_ping")
        } else if current.id.name == "FacetNotExist" {
            throw Ice.FacetNotExistException(id: current.id, facet: "", operation: "ice_ping")
        }
    }

    func getName(current: Ice.Current) throws -> String {
        if current.id.name == "ObjectNotExist" {
            throw Ice.ObjectNotExistException(id: current.id, facet: "", operation: "ice_ping")
        } else if current.id.name == "FacetNotExist" {
            throw Ice.FacetNotExistException(id: current.id, facet: "", operation: "ice_ping")
        }
        return current.id.name
    }
}

func allTests(_ helper: TestHelper) throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    let output = helper.getWriter()

    let oa = try communicator.createObjectAdapterWithEndpoints(name: "MyOA", endpoints: "tcp -h localhost")
    try oa.activate()

    let servant = MyObjectI()

    //
    // Register default servant with category "foo"
    //
    try oa.addDefaultServant(servant: MyObjectDisp(servant), category: "foo")

    //
    // Start test
    //
    output.write("testing single category... ")

    var r = oa.findDefaultServant("foo")
    try test((r as! MyObjectDisp).servant as? MyObjectI === servant)

    r = oa.findDefaultServant("bar")
    try test(r == nil)

    var identity = Ice.Identity()
    identity.category = "foo"

    let names = ["foo", "bar", "x", "y", "abcdefg"]

    var prx: MyObjectPrx!
    for name in names {
        identity.name = name
        prx = try uncheckedCast(prx: oa.createProxy(identity), type: MyObjectPrx.self)
        try prx.ice_ping()
        try test(prx.getName() == name)
    }

    identity.name = "ObjectNotExist"
    prx = try uncheckedCast(prx: oa.createProxy(identity), type: MyObjectPrx.self)
    do {
        try prx.ice_ping()
        try test(false)
    } catch is Ice.ObjectNotExistException {} // Expected

    do {
        _ = try prx.getName()
        try test(false)
    } catch is Ice.ObjectNotExistException {} // Expected

    identity.name = "FacetNotExist"
    prx = try uncheckedCast(prx: oa.createProxy(identity), type: MyObjectPrx.self)
    do {
        try prx.ice_ping()
        try test(false)
    } catch is Ice.FacetNotExistException {} // Expected

    do {
        _ = try prx.getName()
        try test(false)
    } catch is Ice.FacetNotExistException {} // Expected

    identity.category = "bar"
    for name in names {
        identity.name = name
        prx = try uncheckedCast(prx: oa.createProxy(identity), type: MyObjectPrx.self)

        do {
            try prx.ice_ping()
            try test(false)
        } catch is Ice.ObjectNotExistException {} // Expected

        do {
            _ = try prx.getName()
            try test(false)
        } catch is Ice.ObjectNotExistException {} // Expected
    }

    _ = try oa.removeDefaultServant("foo")
    identity.category = "foo"
    prx = try uncheckedCast(prx: oa.createProxy(identity), type: MyObjectPrx.self)
    do {
        try prx.ice_ping()
        try test(false)
    } catch is Ice.ObjectNotExistException {} // Expected

    output.writeLine("ok")

    output.write("testing default category... ")

    try oa.addDefaultServant(servant: MyObjectDisp(servant), category: "")

    r = oa.findDefaultServant("bar")
    try test(r == nil)

    r = oa.findDefaultServant("")
    try test((r as! MyObjectDisp).servant as? MyObjectI === servant)

    for name in names {
        identity.name = name
        prx = try uncheckedCast(prx: oa.createProxy(identity), type: MyObjectPrx.self)
        try prx.ice_ping()
        try test(prx.getName() == name)
    }
    output.writeLine("ok")
}
