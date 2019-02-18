// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Ice
import Foundation

protocol HelloPrx: Ice.ObjectPrx {

}

extension HelloPrx {
    func sayHello(timeout iceP_timeout: Int32) throws {
        let impl = self as! _HelloPrxI
        let os = impl._createOutputStream()
        os.startEncapsulation()
        iceP_timeout.ice_write(to: os)
        os.endEncapsulation()
        let _ = try impl._invoke(operation: "sayHello",
                                 mode: OperationMode.Idempotent,
                                 twowayOnly: false,
                                 inParams: os,
                                 hasOutParams: false)
    }
}

class _HelloPrxI: Ice._ObjectPrxI, HelloPrx {
    override class func ice_staticId() -> String {
        return "::Demo::Hello"
    }
}

func checkedCast(prx: ObjectPrx, type: HelloPrx.Protocol, facet: String? = nil, context: Context? = nil) throws -> HelloPrx? {
    return try _HelloPrxI.checkedCast(prx: prx, facet: facet, context: context) as _HelloPrxI?
}

func uncheckedCast(prx: ObjectPrx, type: HelloPrx.Protocol, facet: String? = nil, context: Context? = nil) -> HelloPrx? {
    return _HelloPrxI.uncheckedCast(prx: prx, facet: facet, context: context) as _HelloPrxI?
}

func ice_staticId(_: HelloPrx.Protocol) -> String {
    return _HelloPrxI.ice_staticId()
}

class MyClass: Ice.Value {

    lazy var foobar: String! = { print("foo"); return "foo" }()

    func ice_getSlicedData() -> SlicedData? {
        return nil
    }

    required init(from ins: Ice.InputStream) throws {

    }
    required init() {

    }

    func ice_id() -> String {
        return "::Demo::MyClass"
    }

    func ice_preMarshal() {
    }
    func ice_postUnmarshal() {
    }

    static func ice_staticId() -> String {
        return "::Demo::MyClass"
    }

    func ice_read(from ins: Ice.InputStream) throws {

    }

    func ice_write(to os: Ice.OutputStream) {

    }

}

extension Ice.ClassResolver {
    @objc dynamic class func _Demo_MyClass() -> AnyClass {
        return MyClass.self
    }
}

func main() throws {
//    let args = ["--Ice.Trace.Network=3", "--Ice.Trace.Protocol=1"]
    let communicator = try Ice.initialize(args: CommandLine.arguments)
//    let communicator = try Ice.initialize(args: args)
    defer {communicator.destroy()}
    let objectPrx: ObjectPrx = try communicator.stringToProxy(str: "hello:default -p 10000 -h localhost")!
    let helloPrx = try checkedCast(prx: objectPrx, type: HelloPrx.self)!
    try helloPrx.sayHello(timeout: 1)

    let connection = try helloPrx.ice_getConnection()!
    let connection2 = try helloPrx.ice_getConnection()!
    precondition(connection === connection2)

//    try connection.setACM(timeout: 200, close: nil, heartbeat: nil)
//
//    let newPrx = try helloPrx.ice_timeout(timeout: 200).ice_compress(compress: false)
//    let compress = newPrx.ice_getCompress()
//    print(compress as Any)
//    let timeout = newPrx.ice_getTimeout()
//    print(timeout as Any)
//
//    let p = try communicator.stringToProxy(str: "")
//    print(p as Any)
//
//    let h: HelloPrx = try helloPrx.ice_endpointSelection(type: EndpointSelectionType.Random)
//    try h.sayHello(timeout: 1)
//    print("foo")
}

do {
//    try main()

//    if let type = Ice.ClassResolver.resolve(typeId: "::Demo::MyClass") as Ice.Value.Type? {
//        let a  = type.init()
//        print(a.ice_id())
//    }

    var foo: Int!

//    foo = 4

   let m = MyClass()

    print(m.foobar == nil)

}
catch let error as LocalException {
    print(error)
}
catch let err {
    print("more errors \(err)")
}
