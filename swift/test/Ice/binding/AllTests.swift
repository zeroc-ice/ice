//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
import Ice
import TestCommon

func createTestIntfPrx(_ adapters: [RemoteObjectAdapterPrx]) throws -> TestIntfPrx {
    var endpoints: [Ice.Endpoint] = []
    var obj: TestIntfPrx! = nil
    for adapter in adapters {
        obj = try adapter.getTestIntf()!
        for e in obj.ice_getEndpoints() {
            endpoints.append(e)
        }
    }
    return obj.ice_endpoints(endpoints)
}

func deactivate(communicator: RemoteCommunicatorPrx, adapters: [RemoteObjectAdapterPrx]) throws {
    for adapter in adapters {
        try communicator.deactivateObjectAdapter(adapter)
    }
}

func allTests(_ helper: TestHelper) throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    let output = helper.getWriter()

    let com = uncheckedCast(prx: try communicator.stringToProxy("communicator:\(helper.getTestEndpoint(num: 0))")!,
                            type: RemoteCommunicatorPrx.self)

    output.write("testing binding with single endpoint... ")

    do {
        let adapter = try com.createObjectAdapter(name: "Adapter", endpoints: "default")!

        let test1 = try adapter.getTestIntf()!
        let test2 = try adapter.getTestIntf()!

        try test(test1.ice_getConnection() === test2.ice_getConnection())

        try test1.ice_ping()
        try test2.ice_ping()

        try com.deactivateObjectAdapter(adapter)

        let test3 = uncheckedCast(prx: test1, type: TestIntfPrx.self)
        try test(test3.ice_getConnection() === test1.ice_getConnection())
        try test(test3.ice_getConnection() === test2.ice_getConnection())

        do {
            try test3.ice_ping()
            try test(false)
        } catch is Ice.ConnectFailedException {
        } catch is Ice.ConnectTimeoutException {}
    }
    output.writeLine("ok")

    output.write("testing binding with multiple endpoints... ")
    do {
        var adapters = [try com.createObjectAdapter(name: "Adapter11", endpoints: "default")!,
                        try com.createObjectAdapter(name: "Adapter12", endpoints: "default")!,
                        try com.createObjectAdapter(name: "Adapter13", endpoints: "default")!]

        //
        // Ensure that when a connection is opened it's reused for new
        // proxies and that all endpoints are eventually tried.
        //
        var names = ["Adapter11", "Adapter12", "Adapter13"]
        while names.count > 0 {
            var adpts = adapters

            let test1 = try createTestIntfPrx(adpts)
            adpts.shuffle()
            let test2 = try createTestIntfPrx(adpts)
            adpts.shuffle()
            let test3 = try createTestIntfPrx(adpts)
            try test1.ice_ping()
            try test(test1.ice_getConnection() === test2.ice_getConnection())
            try test(test2.ice_getConnection() === test3.ice_getConnection())

            let adapterName = try test1.getAdapterName()
            names.removeAll(where: { $0 == adapterName})
            try test1.ice_getConnection()!.close(.GracefullyWithWait)
        }

        //
        // Ensure that the proxy correctly caches the connection(we
        // always send the request over the same connection.)
        //
        do {
            for adpt in adapters {
                try adpt.getTestIntf()!.ice_ping()
            }

            let t = try createTestIntfPrx(adapters)
            let name = try t.getAdapterName()
            let nRetry = 10
            var i = 0

            while try i < nRetry && t.getAdapterName() == name {
                i += 1
            }
            try test(i == nRetry)

            for adpt in adapters {
                try adpt.getTestIntf()!.ice_getConnection()!.close(.GracefullyWithWait)
            }
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapters.
        //
        try com.deactivateObjectAdapter(adapters[0])
        names.append("Adapter12")
        names.append("Adapter13")

        while names.count > 0 {
            var adpts = adapters

            let test1 = try createTestIntfPrx(adpts)
            adpts.shuffle()
            let test2 = try createTestIntfPrx(adpts)
            adpts.shuffle()
            let test3 = try createTestIntfPrx(adpts)

            try test(test1.ice_getConnection() === test2.ice_getConnection())
            try test(test2.ice_getConnection() === test3.ice_getConnection())

            let adapterName = try test1.getAdapterName()
            names.removeAll(where: { $0 == adapterName})

            try test1.ice_getConnection()!.close(.GracefullyWithWait)
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapter.
        //
        try com.deactivateObjectAdapter(adapters[2])
        let obj = try createTestIntfPrx(adapters)
        try test(obj.getAdapterName() == "Adapter12")

        try deactivate(communicator: com, adapters: adapters)
    }
    output.writeLine("ok")

    output.write("testing binding with multiple random endpoints... ")
    do {
        var adapters = [ try com.createObjectAdapter(name: "AdapterRandom11", endpoints: "default")!,
                         try com.createObjectAdapter(name: "AdapterRandom12", endpoints: "default")!,
                         try com.createObjectAdapter(name: "AdapterRandom13", endpoints: "default")!,
                         try com.createObjectAdapter(name: "AdapterRandom14", endpoints: "default")!,
                         try com.createObjectAdapter(name: "AdapterRandom15", endpoints: "default")!]

        var count: Int32 = 20
        var adapterCount = adapters.count
        while count > 0 {
            count -= 1
            var proxies: [TestIntfPrx] = []
            if count == 1 {
                try com.deactivateObjectAdapter(adapters[4])
                adapterCount -= 1
            }

            for _ in 0..<10 {
                var adpts: [RemoteObjectAdapterPrx] = []
                for _ in 0..<Int.random(in: 1...adapters.count) {
                    adpts.append(adapters.randomElement()!)
                }
                proxies.append(try createTestIntfPrx(adpts))
            }

            for prx in proxies {
                _ = prx.getAdapterNameAsync()
            }

            for prx in proxies {
                do {
                    try prx.ice_ping()
                } catch is Ice.LocalException {}
            }

            var connections: [Ice.Connection] = []
            for prx in proxies {
                if let conn = prx.ice_getCachedConnection() {
                    if !connections.contains(where: { $0 === conn }) {
                        connections.append(conn)
                    }
                }
            }
            try test(connections.count <= adapterCount)

            for a in adapters {
                do {
                    try a.getTestIntf()!.ice_getConnection()!.close(.GracefullyWithWait)
                } catch is Ice.LocalException {} // Expected if adapter is down.
            }
        }
    }
    output.writeLine("ok")

    output.write("testing binding with multiple endpoints and AMI... ")
    do {
        var adapters = [try com.createObjectAdapter(name: "AdapterAMI11", endpoints: "default")!,
                        try com.createObjectAdapter(name: "AdapterAMI12", endpoints: "default")!,
                        try com.createObjectAdapter(name: "AdapterAMI13", endpoints: "default")!]

        //
        // Ensure that when a connection is opened it's reused for new
        // proxies and that all endpoints are eventually tried.
        //
        var names = ["AdapterAMI11", "AdapterAMI12", "AdapterAMI13"]
        while names.count > 0 {
            var adpts = adapters

            let test1 = try createTestIntfPrx(adpts)
            adpts.shuffle()
            let test2 = try createTestIntfPrx(adpts)
            adpts.shuffle()
            let test3 = try createTestIntfPrx(adpts)
            try test1.ice_ping()
            try test(test1.ice_getConnection() === test2.ice_getConnection())
            try test(test2.ice_getConnection() === test3.ice_getConnection())

            let adapterName = try test1.getAdapterName()
            names.removeAll(where: { $0 == adapterName})
            try test1.ice_getConnection()!.close(.GracefullyWithWait)
        }

        //
        // Ensure that the proxy correctly caches the connection(we
        // always send the request over the same connection.)
        //
        do {
            for adpt in adapters {
                try adpt.getTestIntf()!.ice_ping()
            }

            let t = try createTestIntfPrx(adapters)
            let name = try t.getAdapterNameAsync().wait()
            let nRetry = 10
            var i = 0
            while try i < nRetry && t.getAdapterNameAsync().wait() == name {
                i += 1
            }
            try test(i == nRetry)

            for adpt in adapters {
                try adpt.getTestIntf()!.ice_getConnection()!.close(.GracefullyWithWait)
            }
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapters.
        //
        try com.deactivateObjectAdapter(adapters[0])
        names.append("AdapterAMI12")
        names.append("AdapterAMI13")
        while names.count > 0 {
            var adpts = adapters
            let test1 = try createTestIntfPrx(adpts)
            adpts.shuffle()
            let test2 = try createTestIntfPrx(adpts)
            adpts.shuffle()
            let test3 = try createTestIntfPrx(adpts)

            try test(test1.ice_getConnection() === test2.ice_getConnection())
            try test(test2.ice_getConnection() === test3.ice_getConnection())

            let adapterName = try test1.getAdapterName()
            names.removeAll(where: { $0 == adapterName})
            try test1.ice_getConnection()!.close(.GracefullyWithWait)
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapter.
        //
        try com.deactivateObjectAdapter(adapters[2])
        let obj = try createTestIntfPrx(adapters)
        try test(obj.getAdapterNameAsync().wait() == "AdapterAMI12")

        try deactivate(communicator: com, adapters: adapters)
    }
    output.writeLine("ok")

    output.write("testing random endpoint selection... ")
    do {
        let adapters = [try com.createObjectAdapter(name: "Adapter21", endpoints: "default")!,
                        try com.createObjectAdapter(name: "Adapter22", endpoints: "default")!,
                        try com.createObjectAdapter(name: "Adapter23", endpoints: "default")!]

        var obj = try createTestIntfPrx(adapters)
        try test(obj.ice_getEndpointSelection() == .Random)

        var names = ["Adapter21", "Adapter22", "Adapter23"]
        while names.count > 0 {
            let adapterName = try obj.getAdapterName()
            names.removeAll(where: { $0 == adapterName})
            try obj.ice_getConnection()!.close(.GracefullyWithWait)
        }

        obj = uncheckedCast(prx: obj.ice_endpointSelection(.Random), type: TestIntfPrx.self)
        try test(obj.ice_getEndpointSelection() == .Random)

        names.append("Adapter21")
        names.append("Adapter22")
        names.append("Adapter23")

        while names.count > 0 {
            let adapterName = try obj.getAdapterName()
            names.removeAll(where: { $0 == adapterName})
            try obj.ice_getConnection()!.close(.GracefullyWithWait)
        }

        try deactivate(communicator: com, adapters: adapters)
    }
    output.writeLine("ok")

    output.write("testing ordered endpoint selection... ")
    do {
        var adapters = [try com.createObjectAdapter(name: "Adapter31", endpoints: "default")!,
                        try com.createObjectAdapter(name: "Adapter32", endpoints: "default")!,
                        try com.createObjectAdapter(name: "Adapter33", endpoints: "default")!]

        var obj = try createTestIntfPrx(adapters)
        obj = uncheckedCast(prx: obj.ice_endpointSelection(.Ordered), type: TestIntfPrx.self)
        try test(obj.ice_getEndpointSelection() == .Ordered)
        let nRetry = 3

        //
        // Ensure that endpoints are tried in order by deactiving the adapters
        // one after the other.
        //
        var i = 0
        while try i < nRetry && obj.getAdapterName() == "Adapter31" {
            i += 1
        }
        try test(i == nRetry)
        try com.deactivateObjectAdapter(adapters[0])

        i = 0
        while try i < nRetry && obj.getAdapterName() == "Adapter32" {
            i += 1
        }
        try test(i == nRetry)
        try com.deactivateObjectAdapter(adapters[1])

        i = 0
        while try i < nRetry && obj.getAdapterName() == "Adapter33" {
            i += 1
        }
        try test(i == nRetry)
        try com.deactivateObjectAdapter(adapters[2])

        do {
            _ = try obj.getAdapterName()
        } catch is Ice.ConnectFailedException {
        } catch is Ice.ConnectTimeoutException {
        }

        var endpoints = obj.ice_getEndpoints()

        adapters.removeAll()

        //
        // Now, re-activate the adapters with the same endpoints in the opposite
        // order.
        //
        try adapters.append(com.createObjectAdapter(name: "Adapter36", endpoints: endpoints[2].toString())!)
        i = 0
        while try i < nRetry && obj.getAdapterName() == "Adapter36" {
            i += 1
        }
        try test(i == nRetry)
        try obj.ice_getConnection()!.close(.GracefullyWithWait)

        try adapters.append(com.createObjectAdapter(name: "Adapter35", endpoints: endpoints[1].toString())!)
        i = 0
        while try i < nRetry && obj.getAdapterName() == "Adapter35" {
            i += 1
        }
        try test(i == nRetry)
        try obj.ice_getConnection()!.close(.GracefullyWithWait)

        try adapters.append(com.createObjectAdapter(name: "Adapter34", endpoints: endpoints[0].toString())!)
        i = 0
        while try i < nRetry && obj.getAdapterName() == "Adapter34" {
            i += 1
        }
        try test(i == nRetry)

        try deactivate(communicator: com, adapters: adapters)
    }
    output.writeLine("ok")

    output.write("testing per request binding with single endpoint... ")
    do {
        let adapter = try com.createObjectAdapter(name: "Adapter41", endpoints: "default")!

        let test1 = try uncheckedCast(prx: adapter.getTestIntf()!.ice_connectionCached(false),
                                      type: TestIntfPrx.self)
        let test2 = try uncheckedCast(prx: adapter.getTestIntf()!.ice_connectionCached(false),
                                      type: TestIntfPrx.self)
        try test(!test1.ice_isConnectionCached())
        try test(!test2.ice_isConnectionCached())
        try test(test1.ice_getConnection() != nil &&
                 test2.ice_getConnection() != nil)
        try test(test1.ice_getConnection() === test2.ice_getConnection())

        try test1.ice_ping()

        try com.deactivateObjectAdapter(adapter)

        let test3 = uncheckedCast(prx: test1, type: TestIntfPrx.self)
        do {
            try test(test3.ice_getConnection() === test1.ice_getConnection())
            try test(false)
        } catch is Ice.ConnectFailedException {
        } catch is Ice.ConnectTimeoutException {}
    }
    output.writeLine("ok")

    output.write("testing per request binding with multiple endpoints... ")
    do {
        var adapters = [try com.createObjectAdapter(name: "Adapter51", endpoints: "default")!,
                        try com.createObjectAdapter(name: "Adapter52", endpoints: "default")!,
                        try com.createObjectAdapter(name: "Adapter53", endpoints: "default")!]

        let obj = try uncheckedCast(prx: createTestIntfPrx(adapters).ice_connectionCached(false),
                                    type: TestIntfPrx.self)
        try test(!obj.ice_isConnectionCached())

        var names = ["Adapter51", "Adapter52", "Adapter53"]
        while names.count > 0 {
            let name = try obj.getAdapterName()
            names.removeAll(where: { $0 == name })
        }

        try com.deactivateObjectAdapter(adapters[0])

        names.append("Adapter52")
        names.append("Adapter53")
        while names.count > 0 {
            let name = try obj.getAdapterName()
            names.removeAll(where: { $0 == name })
        }

        try com.deactivateObjectAdapter(adapters[2])

        try test(obj.getAdapterName() == "Adapter52")

        try deactivate(communicator: com, adapters: adapters)
    }
    output.writeLine("ok")

    output.write("testing per request binding with multiple endpoints and AMI... ")
    do {
        var adapters = [try com.createObjectAdapter(name: "AdapterAMI51", endpoints: "default")!,
                        try com.createObjectAdapter(name: "AdapterAMI52", endpoints: "default")!,
                        try com.createObjectAdapter(name: "AdapterAMI53", endpoints: "default")!]

        let obj = try uncheckedCast(prx: createTestIntfPrx(adapters).ice_connectionCached(false),
                                    type: TestIntfPrx.self)
        try test(!obj.ice_isConnectionCached())

        var names = ["AdapterAMI51", "AdapterAMI52", "AdapterAMI53"]
        while names.count > 0 {
            let adapterName = try obj.getAdapterNameAsync().wait()
            names.removeAll(where: { adapterName == $0 })
        }

        try com.deactivateObjectAdapter(adapters[0])

        names.append("AdapterAMI52")
        names.append("AdapterAMI53")
        while names.count > 0 {
            let adapterName = try obj.getAdapterNameAsync().wait()
            names.removeAll(where: { adapterName == $0 })
        }

        try com.deactivateObjectAdapter(adapters[2])

        try test(obj.getAdapterNameAsync().wait() == "AdapterAMI52")

        try deactivate(communicator: com, adapters: adapters)
    }
    output.writeLine("ok")

    output.write("testing per request binding and ordered endpoint selection... ")
    do {
        var adapters = [try com.createObjectAdapter(name: "Adapter61", endpoints: "default")!,
                        try com.createObjectAdapter(name: "Adapter62", endpoints: "default")!,
                        try com.createObjectAdapter(name: "Adapter63", endpoints: "default")!]

        var obj = try createTestIntfPrx(adapters)
        obj = uncheckedCast(prx: obj.ice_endpointSelection(.Ordered),
                            type: TestIntfPrx.self)
        try test(obj.ice_getEndpointSelection() == .Ordered)
        obj = uncheckedCast(prx: obj.ice_connectionCached(false), type: TestIntfPrx.self)
        try test(!obj.ice_isConnectionCached())
        let nRetry = 3
        var i = 0
        //
        // Ensure that endpoints are tried in order by deactiving the adapters
        // one after the other.
        //
        while try i < nRetry && obj.getAdapterName() == "Adapter61" {
            i += 1
        }
        try test(i == nRetry)
        try com.deactivateObjectAdapter(adapters[0])

        i = 0
        while try i < nRetry && obj.getAdapterName() == "Adapter62" {
            i += 1
        }
        try test(i == nRetry)
        try com.deactivateObjectAdapter(adapters[1])

        i = 0
        while try i < nRetry && obj.getAdapterName() == "Adapter63" {
            i += 1
        }
        try test(i == nRetry)
        try com.deactivateObjectAdapter(adapters[2])

        do {
            _ = try obj.getAdapterName()
        } catch is Ice.ConnectFailedException {
        } catch is Ice.ConnectTimeoutException {
        }

        let endpoints = obj.ice_getEndpoints()

        adapters.removeAll()

        //
        // Now, re-activate the adapters with the same endpoints in the opposite
        // order.
        //
        try adapters.append(com.createObjectAdapter(name: "Adapter66", endpoints: endpoints[2].toString())!)
        i = 0
        while try i < nRetry && obj.getAdapterName() == "Adapter66" {
            i += 1
        }
        try test(i == nRetry)

        try adapters.append(com.createObjectAdapter(name: "Adapter65", endpoints: endpoints[1].toString())!)
        i = 0
        while try i < nRetry && obj.getAdapterName() == "Adapter65" {
            i += 1
        }
        try test(i == nRetry)

        try adapters.append(com.createObjectAdapter(name: "Adapter64", endpoints: endpoints[0].toString())!)
        i = 0
        while try i < nRetry && obj.getAdapterName() == "Adapter64" {
            i += 1
        }
        try test(i == nRetry)

        try deactivate(communicator: com, adapters: adapters)
    }
    output.writeLine("ok")

    output.write("testing per request binding and ordered endpoint selection and AMI... ")
    do {
        var adapters = [try com.createObjectAdapter(name: "AdapterAMI61", endpoints: "default")!,
                        try com.createObjectAdapter(name: "AdapterAMI62", endpoints: "default")!,
                        try com.createObjectAdapter(name: "AdapterAMI63", endpoints: "default")!]

        var obj = try createTestIntfPrx(adapters)
        obj = uncheckedCast(prx: obj.ice_endpointSelection(.Ordered), type: TestIntfPrx.self)
        try test(obj.ice_getEndpointSelection() == .Ordered)
        obj = uncheckedCast(prx: obj.ice_connectionCached(false), type: TestIntfPrx.self)
        try test(!obj.ice_isConnectionCached())

        let nRetry = 3
        var i = 0

        //
        // Ensure that endpoints are tried in order by deactiving the adapters
        // one after the other.
        //
        while try i < nRetry && obj.getAdapterNameAsync().wait() == "AdapterAMI61" {
            i += 1
        }
        try test(i == nRetry)
        try com.deactivateObjectAdapter(adapters[0])

        while try i < nRetry && obj.getAdapterNameAsync().wait() == "AdapterAMI62" {
            i += 1
        }
        try test(i == nRetry)
        try com.deactivateObjectAdapter(adapters[1])

        i = 0
        while try i < nRetry && obj.getAdapterNameAsync().wait() == "AdapterAMI63" {
            i += 1
        }
        try test(i == nRetry)
        try com.deactivateObjectAdapter(adapters[2])

        do {
            _ = try obj.getAdapterName()
        } catch is Ice.ConnectFailedException {
        } catch is Ice.ConnectTimeoutException {}

        let endpoints = obj.ice_getEndpoints()

        adapters.removeAll()

        //
        // Now, re-activate the adapters with the same endpoints in the opposite
        // order.
        //
        try adapters.append(com.createObjectAdapter(name: "AdapterAMI66", endpoints: endpoints[2].toString())!)
        i = 0
        while try i < nRetry && obj.getAdapterNameAsync().wait() == "AdapterAMI66" {
            i += 1
        }
        try test(i == nRetry)

        try adapters.append(com.createObjectAdapter(name: "AdapterAMI65", endpoints: endpoints[1].toString())!)
        i = 0
        while try i < nRetry && obj.getAdapterNameAsync().wait() == "AdapterAMI65" {
            i += 1
        }
        try test(i == nRetry)

        try adapters.append(com.createObjectAdapter(name: "AdapterAMI64", endpoints: endpoints[0].toString())!)
        i = 0
        while try i < nRetry && obj.getAdapterNameAsync().wait() == "AdapterAMI64" {
            i += 1
        }
        try test(i == nRetry)

        try deactivate(communicator: com, adapters: adapters)
    }
    output.writeLine("ok")

    output.write("testing endpoint mode filtering... ")
    do {
        let adapters = [try com.createObjectAdapter(name: "Adapter71", endpoints: "default")!,
                        try com.createObjectAdapter(name: "Adapter72", endpoints: "udp")!]

        let obj = try createTestIntfPrx(adapters)
        try test(obj.getAdapterName() == "Adapter71")

        let testUDP = uncheckedCast(prx: obj.ice_datagram(), type: TestIntfPrx.self)
        try test(obj.ice_getConnection() !== testUDP.ice_getConnection())
        do {
            _ = try testUDP.getAdapterName()
        } catch is Ice.TwowayOnlyException {}
    }
    output.writeLine("ok")

    if communicator.getProperties().getProperty("Ice.Plugin.IceSSL") != "" {
        output.write("testing unsecure vs. secure endpoints... ")
        do {
            var adapters = [try com.createObjectAdapter(name: "Adapter81", endpoints: "ssl")!,
                            try com.createObjectAdapter(name: "Adapter82", endpoints: "tcp")!]

            let obj = try createTestIntfPrx(adapters)
            for _ in 0..<5 {
                try test(obj.getAdapterName() == "Adapter82")
                try obj.ice_getConnection()!.close(.GracefullyWithWait)
            }

            var testSecure = uncheckedCast(prx: obj.ice_secure(true), type: TestIntfPrx.self)
            try test(testSecure.ice_isSecure())
            testSecure = uncheckedCast(prx: obj.ice_secure(false), type: TestIntfPrx.self)
            try test(!testSecure.ice_isSecure())
            testSecure = uncheckedCast(prx: obj.ice_secure(true), type: TestIntfPrx.self)
            try test(testSecure.ice_isSecure())
            try test(obj.ice_getConnection() !== testSecure.ice_getConnection())

            try com.deactivateObjectAdapter(adapters[1])

            for _ in 0..<5 {
                try test(obj.getAdapterName() == "Adapter81")
                try obj.ice_getConnection()!.close(.GracefullyWithWait)
            }

            // Reactive tcp OA.
            _ = try com.createObjectAdapter(name: "Adapter83", endpoints: obj.ice_getEndpoints()[1].toString())

            for _ in 0..<5 {
                try test(obj.getAdapterName() == "Adapter83")
                try obj.ice_getConnection()!.close(.GracefullyWithWait)
            }

            try com.deactivateObjectAdapter(adapters[0])
            do {
                try testSecure.ice_ping()
                try test(false)
            } catch is Ice.ConnectFailedException {
            } catch is Ice.ConnectTimeoutException {
            }

            try deactivate(communicator: com, adapters: adapters)
        }
        output.writeLine("ok")
    }

    do {
        output.write("testing ipv4 & ipv6 connections... ")

        let ipv4 = Ice.createProperties()
        ipv4.setProperty(key: "Ice.IPv4", value: "1")
        ipv4.setProperty(key: "Ice.IPv6", value: "0")
        ipv4.setProperty(key: "Adapter.Endpoints", value: "tcp -h localhost")

        let ipv6 = Ice.createProperties()
        ipv6.setProperty(key: "Ice.IPv4", value: "0")
        ipv6.setProperty(key: "Ice.IPv6", value: "1")
        ipv6.setProperty(key: "Adapter.Endpoints", value: "tcp -h localhost")

        let bothPreferIPv4 = Ice.createProperties()
        bothPreferIPv4.setProperty(key: "Ice.IPv4", value: "1")
        bothPreferIPv4.setProperty(key: "Ice.IPv6", value: "1")
        bothPreferIPv4.setProperty(key: "Ice.PreferIPv6Address", value: "0")
        bothPreferIPv4.setProperty(key: "Adapter.Endpoints", value: "tcp -h localhost")

        let bothPreferIPv6 = Ice.createProperties()
        bothPreferIPv6.setProperty(key: "Ice.IPv4", value: "1")
        bothPreferIPv6.setProperty(key: "Ice.IPv6", value: "1")
        bothPreferIPv6.setProperty(key: "Ice.PreferIPv6Address", value: "1")
        bothPreferIPv6.setProperty(key: "Adapter.Endpoints", value: "tcp -h localhost")

        let clientProps = [ipv4, ipv6, bothPreferIPv4, bothPreferIPv6]

        let endpoint = "tcp -p \(helper.getTestPort(num: 2))"

        let anyipv4 = ipv4.clone()
        anyipv4.setProperty(key: "Adapter.Endpoints", value: endpoint)
        anyipv4.setProperty(key: "Adapter.PublishedEndpoints", value: "\(endpoint) -h 127.0.0.1")

        let anyipv6 = ipv6.clone()
        anyipv6.setProperty(key: "Adapter.Endpoints", value: endpoint)
        anyipv6.setProperty(key: "Adapter.PublishedEndpoints", value: "\(endpoint) -h \".1\"")

        let anyboth = Ice.createProperties()
        anyboth.setProperty(key: "Ice.IPv4", value: "1")
        anyboth.setProperty(key: "Ice.IPv6", value: "1")
        anyboth.setProperty(key: "Adapter.Endpoints", value: endpoint)
        anyboth.setProperty(key: "Adapter.PublishedEndpoints",
                            value: "\(endpoint) -h \"::1\":\(endpoint) -h 127.0.0.1")

        let localipv4 = ipv4.clone()
        localipv4.setProperty(key: "Adapter.Endpoints", value: "tcp -h 127.0.0.1")

        let localipv6 = ipv6.clone()
        localipv6.setProperty(key: "Adapter.Endpoints", value: "tcp -h \"::1\"")

        let serverProps = [anyipv4, anyipv6, anyboth, localipv4, localipv6]

        var ipv6NotSupported = false
        for  p in serverProps {
            var serverInitData = Ice.InitializationData()
            serverInitData.properties = p
            let serverCommunicator = try Ice.initialize(serverInitData)
            let oa: Ice.ObjectAdapter
            do {
                oa = try serverCommunicator.createObjectAdapter("Adapter")
                try oa.activate()
            } catch is Ice.DNSException {
                serverCommunicator.destroy()
                continue // IP version not supported.
            } catch is Ice.SocketException {
                if p === ipv6 {
                    ipv6NotSupported = true
                }
                serverCommunicator.destroy()
                continue // IP version not supported.
            }

            var prx = try oa.createProxy(Ice.stringToIdentity("dummy"))
            do {
                try prx.ice_collocationOptimized(false).ice_ping()
            } catch is Ice.LocalException {
                serverCommunicator.destroy()
                continue // IP version not supported.
            }

            let strPrx = prx.ice_toString()
            for q in clientProps {
                var clientInitData = Ice.InitializationData()
                clientInitData.properties = q
                let clientCommunicator = try Ice.initialize(clientInitData)
                prx = try clientCommunicator.stringToProxy(strPrx)!
                do {
                    try prx.ice_ping()
                    try test(false)
                } catch is Ice.ObjectNotExistException {
                    // Expected, no object registered.
                } catch is Ice.DNSException {
                    // Expected if no IPv4 or IPv6 address is
                    // associated to localhost or if trying to connect
                    // to an any endpoint with the wrong IP version,
                    // e.g.: resolving an IPv4 address when only IPv6
                    // is enabled fails with a DNS exception.
                } catch is Ice.SocketException {
                    try test(
                        (p === ipv4 && q === ipv6) || (p === ipv6 && q === ipv4) ||
                        (p === bothPreferIPv4 && q === ipv6) || (p === bothPreferIPv6 && q === ipv4) ||
                        (p === bothPreferIPv6 && q === ipv6 && ipv6NotSupported) ||
                        (p === anyipv4 && q === ipv6) || (p === anyipv6 && q === ipv4) ||
                        (p === localipv4 && q === ipv6) || (p === localipv6 && q === ipv4) ||
                        (p === ipv6 && q === bothPreferIPv4) || (p === ipv6 && q === bothPreferIPv6) ||
                        (p === bothPreferIPv6 && q === ipv6))
                }
                clientCommunicator.destroy()
            }
            serverCommunicator.destroy()
        }

        output.writeLine("ok")
        try com.shutdown()
    }
}
