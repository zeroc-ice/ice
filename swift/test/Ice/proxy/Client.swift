//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        let writer = getWriter()
        writer.write("testing stringToProxy... ")

        let (communicator, _) = try self.initialize(args: args)
        defer {
            communicator.destroy()
        }
        let rf = "test:\(self.getTestEndpoint(num: 0))"
        let baseProxy = try communicator.stringToProxy(str: rf)
        try test(baseProxy != nil)
        var b1 = try communicator.stringToProxy(str: "test")
        try test(b1!.ice_getIdentity().name == "test" &&
                 b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getAdapterId().isEmpty &&
                 b1!.ice_getFacet().isEmpty)
        b1 = try communicator.stringToProxy(str: "test ")
        try test(b1!.ice_getIdentity().name == "test" &&
                 b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getAdapterId().isEmpty &&
                 b1!.ice_getFacet().isEmpty)
        b1 = try communicator.stringToProxy(str: " test ")
        try test(b1!.ice_getIdentity().name == "test" &&
                 b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getAdapterId().isEmpty &&
                 b1!.ice_getFacet().isEmpty)
        b1 = try communicator.stringToProxy(str: " test")
        try test(b1!.ice_getIdentity().name == "test" &&
            b1!.ice_getIdentity().category.isEmpty &&
            b1!.ice_getAdapterId().isEmpty &&
            b1!.ice_getFacet().isEmpty)
        b1 = try communicator.stringToProxy(str: "'test -f facet'")
        try test(b1!.ice_getIdentity().name == "test -f facet" &&
                 b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getFacet().isEmpty)

        do {
            b1 = try communicator.stringToProxy(str: "\"test -f facet'")
            try test(false)
        } catch is Ice.ProxyParseException {}

        b1 = try communicator.stringToProxy(str: "\"test -f facet\"")
        try test(b1!.ice_getIdentity().name == "test -f facet" &&
                 b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getFacet().isEmpty)

        b1 = try communicator.stringToProxy(str: "\"test -f facet@test\"")
        try test(b1!.ice_getIdentity().name == "test -f facet@test" &&
                 b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getFacet().isEmpty)

        b1 = try communicator.stringToProxy(str: "\"test -f facet@test @test\"")
        try test(b1!.ice_getIdentity().name == "test -f facet@test @test" &&
                 b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getFacet().isEmpty)

        do {
            b1 = try communicator.stringToProxy(str: "test test")
            try test(false)
        } catch is Ice.ProxyParseException {}

        b1 = try communicator.stringToProxy(str: "test\\040test")
        try test(b1!.ice_getIdentity().name == "test test" &&
                 b1!.ice_getIdentity().category.isEmpty)
        do {
            b1 = try communicator.stringToProxy(str: "test\\777")
            try test(false)
        } catch is Ice.IdentityParseException {}

        b1 = try communicator.stringToProxy(str: "test\\40test")
        try test(b1!.ice_getIdentity().name == "test test")

        // Test some octal and hex corner cases.
        b1 = try communicator.stringToProxy(str: "test\\4test")
        try test(b1!.ice_getIdentity().name == "test\u{0004}test")
        b1 = try communicator.stringToProxy(str: "test\\04test")
        try test(b1!.ice_getIdentity().name == "test\u{0004}test")
        b1 = try communicator.stringToProxy(str: "test\\004test")
        try test(b1!.ice_getIdentity().name == "test\u{0004}test")
        b1 = try communicator.stringToProxy(str: "test\\1114test")
        try test(b1!.ice_getIdentity().name == "test\u{0049}4test")

        b1 = try communicator.stringToProxy(str: "test\\b\\f\\n\\r\\t\\'\\\"\\\\test")
        try test(b1!.ice_getIdentity().name == "test\u{0008}\u{000C}\n\r\t\'\"\\test" &&
                 b1!.ice_getIdentity().category.isEmpty)

        b1 = try communicator.stringToProxy(str: "category/test")
        try test(b1!.ice_getIdentity().name == "test" &&
                 b1!.ice_getIdentity().category == "category" &&
                 b1!.ice_getAdapterId().isEmpty)

        b1 = try communicator.stringToProxy(str: "")
        try test(b1 == nil)

        b1 = try communicator.stringToProxy(str: "\"\"")
        try test(b1 == nil)

        do {
            b1 = try communicator.stringToProxy(str: "\"\" test"); // Invalid trailing characters.
            try test(false)
        } catch is Ice.ProxyParseException {}

        do {
            b1 = try communicator.stringToProxy(str: "test:") // Missing endpoint.
            try test(false)
        } catch is Ice.EndpointParseException {}

        b1 = try communicator.stringToProxy(str: "test@adapter")
        try test(b1!.ice_getIdentity().name == "test" &&
                 b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getAdapterId() == "adapter")

        do {
            b1 = try communicator.stringToProxy(str: "id@adapter test")
            try test(false)
        } catch is Ice.ProxyParseException {}

        b1 = try communicator.stringToProxy(str: "category/test@adapter")
        try test(b1!.ice_getIdentity().name == "test" &&
                 b1!.ice_getIdentity().category == "category" &&
                 b1!.ice_getAdapterId() == "adapter")
        b1 = try communicator.stringToProxy(str: "category/test@adapter:tcp")
        try test(b1!.ice_getIdentity().name == "test" &&
                 b1!.ice_getIdentity().category == "category" &&
                 b1!.ice_getAdapterId() == "adapter:tcp")
        b1 = try communicator.stringToProxy(str: "'category 1/test'@adapter")
        try test(b1!.ice_getIdentity().name == "test" &&
                 b1!.ice_getIdentity().category == "category 1" &&
                 b1!.ice_getAdapterId() == "adapter")
        b1 = try communicator.stringToProxy(str: "'category/test 1'@adapter")
        try test(b1!.ice_getIdentity().name == "test 1" &&
                 b1!.ice_getIdentity().category == "category" &&
                 b1!.ice_getAdapterId() == "adapter")
        b1 = try communicator.stringToProxy(str: "'category/test'@'adapter 1'")
        try test(b1!.ice_getIdentity().name == "test" &&
                 b1!.ice_getIdentity().category == "category" &&
                 b1!.ice_getAdapterId() == "adapter 1")
        b1 = try communicator.stringToProxy(str: "\"category \\/test@foo/test\"@adapter")
        try test(b1!.ice_getIdentity().name == "test" &&
                 b1!.ice_getIdentity().category == "category /test@foo" &&
                 b1!.ice_getAdapterId() == "adapter")
        b1 = try communicator.stringToProxy(str: "\"category \\/test@foo/test\"@\"adapter:tcp\"")
        try test(b1!.ice_getIdentity().name == "test" &&
                 b1!.ice_getIdentity().category == "category /test@foo" &&
                 b1!.ice_getAdapterId() == "adapter:tcp")

        b1 = try communicator.stringToProxy(str: "id -f facet")
        try test(b1!.ice_getIdentity().name == "id" &&
                 b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getFacet() == "facet")
        b1 = try communicator.stringToProxy(str: "id -f 'facet x'")
        try test(b1!.ice_getIdentity().name == "id" &&
                 b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getFacet() == "facet x")
        b1 = try communicator.stringToProxy(str: "id -f \"facet x\"")
        try test(b1!.ice_getIdentity().name == "id" &&
                 b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getFacet() == "facet x")

        do {
            b1 = try communicator.stringToProxy(str: "id -f \"facet x")
            try test(false)
        } catch is Ice.ProxyParseException {}

        do {
            b1 = try communicator.stringToProxy(str: "id -f \'facet x")
            try test(false)
        } catch is Ice.ProxyParseException {}

        b1 = try communicator.stringToProxy(str: "test -f facet:tcp")
        try test(b1!.ice_getIdentity().name == "test" &&
                 b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getFacet() == "facet" &&
                 b1!.ice_getAdapterId().isEmpty)
        b1 = try communicator.stringToProxy(str: "test -f \"facet:tcp\"")
        try test(b1!.ice_getIdentity().name == "test" &&
                 b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getFacet() == "facet:tcp" &&
                 b1!.ice_getAdapterId().isEmpty)
        b1 = try communicator.stringToProxy(str: "test -f facet@test")
        try test(b1!.ice_getIdentity().name == "test" &&
                   b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getFacet() == "facet" &&
                 b1!.ice_getAdapterId() == "test")
        b1 = try communicator.stringToProxy(str: "test -f 'facet@test'")
        try test(b1!.ice_getIdentity().name == "test" &&
                 b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getFacet() == "facet@test" &&
                 b1!.ice_getAdapterId().isEmpty)
        b1 = try communicator.stringToProxy(str: "test -f 'facet@test'@test")
        try test(b1!.ice_getIdentity().name == "test" &&
                   b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getFacet() == "facet@test" &&
                 b1!.ice_getAdapterId() == "test")

        do {
            b1 = try communicator.stringToProxy(str: "test -f facet@test @test")
            try test(false)
        } catch is Ice.ProxyParseException {}
        b1 = try communicator.stringToProxy(str: "test")
        try test(b1!.ice_isTwoway())
        b1 = try communicator.stringToProxy(str: "test -t")
        try test(b1!.ice_isTwoway())
        b1 = try communicator.stringToProxy(str: "test -o")
        try test(b1!.ice_isOneway())
        b1 = try communicator.stringToProxy(str: "test -O")
        try test(b1!.ice_isBatchOneway())
        b1 = try communicator.stringToProxy(str: "test -d")
        try test(b1!.ice_isDatagram())
        b1 = try communicator.stringToProxy(str: "test -D")
        try test(b1!.ice_isBatchDatagram())
        b1 = try communicator.stringToProxy(str: "test")
        try test(!b1!.ice_isSecure())
        b1 = try communicator.stringToProxy(str: "test -s")
        try test(b1!.ice_isSecure())

        try test(b1!.ice_getEncodingVersion() == Ice.currentEncoding)

        b1 = try communicator.stringToProxy(str: "test -e 1.0")
        try test(b1!.ice_getEncodingVersion().major == 1 &&
                 b1!.ice_getEncodingVersion().minor == 0)

        b1 = try communicator.stringToProxy(str: "test -e 6.5")
        try test(b1!.ice_getEncodingVersion().major == 6 &&
                 b1!.ice_getEncodingVersion().minor == 5)

        b1 = try communicator.stringToProxy(str: "test -p 1.0 -e 1.0")
        try test(b1!.ice_toString() == "test -t -e 1.0")

        b1 = try communicator.stringToProxy(str: "test -p 6.5 -e 1.0")
        try test(b1!.ice_toString() == "test -t -p 6.5 -e 1.0")

        do {
            _ = try communicator.stringToProxy(str: "test:tcp@adapterId")
            try test(false)
        } catch is Ice.EndpointParseException {}

        do {
            _ = try communicator.stringToProxy(str: "test: :tcp")
            try test(false)
        } catch is Ice.EndpointParseException {}

        //
        // Test invalid endpoint syntax
        //
        // TODO enable this once Ice.ObjectAdapter is ported to Swift
        /*do {
            _ = try communicator.createObjectAdapterWithEndpoints(name: "BadAdapter", endpoints: " : ")
            try test(false)
        } catch is Ice.EndpointParseException {}

        do {
            _ = try communicator.createObjectAdapterWithEndpoints(name: "BadAdapter", endpoints: "tcp: ")
            try test(false)
        } catch is Ice.EndpointParseException {}

        do {
            _ = try communicator.createObjectAdapterWithEndpoints(name: "BadAdapter", endpoints: ":tcp")
            try test(false)
        } catch is Ice.EndpointParseException {}*/

        //
        // Test for bug ICE-5543: escaped escapes in stringToIdentity
        //
        var id = Ice.Identity(name: "test", category: ",X2QNUAzSBcJ_e$AV;E\\")
        var id2 = try Ice.stringToIdentity(string: communicator.identityToString(ident: id))
        try test(id == id2)

        id = Ice.Identity(name: "test", category: ",X2QNUAz\\SB\\/cJ_e$AV;E\\\\")
        id2 = try Ice.stringToIdentity(string: communicator.identityToString(ident: id))
        try test(id == id2)

        id = Ice.Identity(name: "/test", category: "cat/")
        var idStr = try communicator.identityToString(ident: id)
        try test(idStr == "cat\\//\\/test")
        id2 = try Ice.stringToIdentity(string: idStr)
        try test(id == id2)

        // Input string with various pitfalls
        id = try Ice.stringToIdentity(string: "\\342\\x82\\254\\60\\x9\\60\\")
        try test(id.name == "€0\t0\\" && id.category == "")

        do {
            // Illegal character < 32
            id = try Ice.stringToIdentity(string: "xx\u{01}FooBar")
            try test(false)
        } catch is Ice.IdentityParseException {}

        do {
            // Illegal surrogate
            id = try Ice.stringToIdentity(string: "xx\\ud911")
            try test(false)
        } catch is Ice.IdentityParseException {}

        // Testing bytes 127(\x7F, \177) and €
        id = Ice.Identity(name: "test", category: "\u{007f}€")

        idStr = try Ice.identityToString(identity: id, mode: Ice.ToStringMode.Unicode)
        try test(idStr == "\\u007f€/test")
        id2 = try Ice.stringToIdentity(string: idStr)
        try test(id == id2)
        try test(Ice.identityToString(identity: id) == idStr)

        idStr = try Ice.identityToString(identity: id, mode: Ice.ToStringMode.ASCII)
        try test(idStr == "\\u007f\\u20ac/test")
        id2 = try Ice.stringToIdentity(string: idStr)
        try test(id == id2)

        idStr = try Ice.identityToString(identity: id, mode: Ice.ToStringMode.Compat)
        try test(idStr == "\\177\\342\\202\\254/test")
        id2 = try Ice.stringToIdentity(string: idStr)
        try test(id == id2)

        id2 = try Ice.stringToIdentity(string: communicator.identityToString(ident: id))
        try test(id == id2)

        // More unicode character
        id = Ice.Identity(name: "banana \u{000E}-\u{1f34c}\u{20ac}\u{00a2}\u{0024}",
                          category: "greek \u{1016a}")

        idStr = try Ice.identityToString(identity: id, mode: Ice.ToStringMode.Unicode)
        try test(idStr == "greek \u{1016a}/banana \\u000e-\u{1f34c}\u{20ac}\u{00a2}$")
        id2 = try Ice.stringToIdentity(string: idStr)
        try test(id == id2)

        idStr = try Ice.identityToString(identity: id, mode: Ice.ToStringMode.ASCII)
        try test(idStr == "greek \\U0001016a/banana \\u000e-\\U0001f34c\\u20ac\\u00a2$")
        id2 = try Ice.stringToIdentity(string: idStr)
        try test(id == id2)

        idStr = try Ice.identityToString(identity: id, mode: Ice.ToStringMode.Compat)
        id2 = try Ice.stringToIdentity(string: idStr)
        try test(idStr == "greek \\360\\220\\205\\252/banana \\016-\\360\\237\\215\\214\\342\\202\\254\\302\\242$")
        try test(id == id2)
        writer.writeLine("ok")

        writer.write("testing proxyToString... ")
        b1 = try communicator.stringToProxy(str: rf)
        var b2 = try communicator.stringToProxy(str: communicator.proxyToString(obj: b1!))

        // TODO Proxy equals
        //try test(b1 == b2)

        if(try b1!.ice_getConnection() != nil) // not colloc-optimized target
        {
            b2 = try b1!.ice_getConnection()!.createProxy(id: Ice.stringToIdentity(string: "fixed"))
            let str = try communicator.proxyToString(obj: b2!)
            try test(b2!.ice_toString() == str)
            let str2 = try b1!.ice_identity(id: b2!.ice_getIdentity()).ice_secure(secure: b2!.ice_isSecure()).ice_toString()
            // Verify that the stringified fixed proxy is the same as a regular stringified proxy
            // but without endpoints
            try test(str2.hasPrefix("\(str):"))
        }
        writer.writeLine("ok")

        writer.write("testing propertyToProxy... ")
        let prop = communicator.getProperties()
        let propertyPrefix = "Foo.Proxy"
        try prop.setProperty(key: propertyPrefix, value: "test:\(self.getTestEndpoint(num: 0))")
        b1 = try communicator.propertyToProxy(property: propertyPrefix)
        try test(b1!.ice_getIdentity().name == "test" &&
                 b1!.ice_getIdentity().category.isEmpty &&
                 b1!.ice_getAdapterId().isEmpty &&
                 b1!.ice_getFacet().isEmpty)

        var property = "\(propertyPrefix).Locator"
        try test(b1!.ice_getLocator() == nil)
        try prop.setProperty(key: property, value: "locator:default -p 10000")
        b1 = try communicator.propertyToProxy(property: propertyPrefix)
        try test(b1!.ice_getLocator() != nil &&
                 b1!.ice_getLocator()!.ice_getIdentity().name == "locator")
        try prop.setProperty(key: property, value: "")

        property = "\(propertyPrefix).LocatorCacheTimeout"
        try test(b1!.ice_getLocatorCacheTimeout() == -1)
        try prop.setProperty(key: property, value: "1")
        b1 = try communicator.propertyToProxy(property: propertyPrefix)
        try test(b1!.ice_getLocatorCacheTimeout() == 1)
        try prop.setProperty(key: property, value: "")

        // Now retest with an indirect proxy.
        try prop.setProperty(key: propertyPrefix, value: "test")
        property = "\(propertyPrefix).Locator"
        try prop.setProperty(key: property, value: "locator:default -p 10000")
        b1 = try communicator.propertyToProxy(property: propertyPrefix)
        try test(b1!.ice_getLocator() != nil &&
                 b1!.ice_getLocator()!.ice_getIdentity().name == "locator")
        try prop.setProperty(key: property, value: "")

        property = "\(propertyPrefix).LocatorCacheTimeout"
        try test(b1!.ice_getLocatorCacheTimeout() == -1)
        try prop.setProperty(key: property, value: "1")
        b1 = try communicator.propertyToProxy(property: propertyPrefix)
        try test(b1!.ice_getLocatorCacheTimeout() == 1)
        try prop.setProperty(key: property, value: "")

        // This cannot be tested so easily because the property is cached
        // on communicator initialization.
        //
        //prop.setProperty("Ice.Default.LocatorCacheTimeout", "60");
        //b1 = communicator.propertyToProxy(propertyPrefix);
        //test(b1.ice_getLocatorCacheTimeout() == 60);
        //prop.setProperty("Ice.Default.LocatorCacheTimeout", "");

        try prop.setProperty(key: propertyPrefix, value: "test:\(self.getTestEndpoint(num: 0))")

        property = "\(propertyPrefix).Router"
        try test(b1!.ice_getRouter() == nil)
        try prop.setProperty(key: property, value: "router:default -p 10000")
        b1 = try communicator.propertyToProxy(property: propertyPrefix)
        try test(b1!.ice_getRouter() != nil &&
                 b1!.ice_getRouter()!.ice_getIdentity().name == "router")
        try prop.setProperty(key: property, value: "")

        property = "\(propertyPrefix).PreferSecure"
        try test(!b1!.ice_isPreferSecure())
        try prop.setProperty(key: property, value: "1")
        b1 = try communicator.propertyToProxy(property: propertyPrefix)
        try test(b1!.ice_isPreferSecure())
        try prop.setProperty(key: property, value: "")

        property = "\(propertyPrefix).ConnectionCached"
        try test(b1!.ice_isConnectionCached())
        try prop.setProperty(key: property, value: "0")
        b1 = try communicator.propertyToProxy(property: propertyPrefix)
        try test(!b1!.ice_isConnectionCached())
        try prop.setProperty(key: property, value: "")

        property = "\(propertyPrefix).InvocationTimeout"
        try test(b1!.ice_getInvocationTimeout() == -1)
        try prop.setProperty(key: property, value: "1000")
        b1 = try communicator.propertyToProxy(property: propertyPrefix)
        try test(b1!.ice_getInvocationTimeout() == 1000)
        try prop.setProperty(key: property, value: "")

        property = "\(propertyPrefix).EndpointSelection"
        try test(b1!.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random)
        try prop.setProperty(key: property, value: "Random");
        b1 = try communicator.propertyToProxy(property: propertyPrefix)
        try test(b1!.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random)
        try prop.setProperty(key: property, value: "Ordered")
        b1 = try communicator.propertyToProxy(property: propertyPrefix)
        try test(b1!.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered)
        try prop.setProperty(key: property, value: "")

        property = "\(propertyPrefix).CollocationOptimized"
        try test(b1!.ice_isCollocationOptimized())
        try prop.setProperty(key: property, value: "0")
        b1 = try communicator.propertyToProxy(property: propertyPrefix)
        try test(!b1!.ice_isCollocationOptimized())
        try prop.setProperty(key: property, value: "")

        property = "\(propertyPrefix).Context.c1"
        try test(b1!.ice_getContext()["c1"] == nil)
        try prop.setProperty(key: property, value: "TEST")
        b1 = try communicator.propertyToProxy(property: propertyPrefix)
        try test(b1!.ice_getContext()["c1"] == "TEST")

        property = "\(propertyPrefix).Context.c2"
        try test(b1!.ice_getContext()["c2"] == nil)
        try prop.setProperty(key: property, value: "TEST")
        b1 = try communicator.propertyToProxy(property: propertyPrefix)
        try test(b1!.ice_getContext()["c2"] == "TEST")

        try prop.setProperty(key: "\(propertyPrefix).Context.c1", value: "")
        try prop.setProperty(key: "\(propertyPrefix).Context.c2", value: "")

        writer.writeLine("ok")

        writer.write("testing proxyToProperty... ")

        b1 = try communicator.stringToProxy(str: "test")
        b1 = try b1!.ice_collocationOptimized(collocated: true)
        b1 = try b1!.ice_connectionCached(cached: true)
        b1 = try b1!.ice_preferSecure(preferSecure: false)
        b1 = try b1!.ice_endpointSelection(type: Ice.EndpointSelectionType.Ordered)
        b1 = try b1!.ice_locatorCacheTimeout(timeout: 100)
        b1 = try b1!.ice_invocationTimeout(timeout: 1234)
        b1 = b1!.ice_encodingVersion(encoding: Ice.EncodingVersion(major: 1, minor: 0))

        var router = try communicator.stringToProxy(str: "router")
        router = try router!.ice_collocationOptimized(collocated: false)
        router = try router!.ice_connectionCached(cached: true)
        router = try router!.ice_preferSecure(preferSecure: true)
        router = try router!.ice_endpointSelection(type: Ice.EndpointSelectionType.Random)
        router = try router!.ice_locatorCacheTimeout(timeout: 200)
        router = try router!.ice_invocationTimeout(timeout: 1500)

        var locator = try communicator.stringToProxy(str: "locator")
        locator = try locator!.ice_collocationOptimized(collocated: true)
        locator = try locator!.ice_connectionCached(cached: false)
        locator = try locator!.ice_preferSecure(preferSecure: true)
        locator = try locator!.ice_endpointSelection(type: Ice.EndpointSelectionType.Random)
        locator = try locator!.ice_locatorCacheTimeout(timeout: 300)
        locator = try locator!.ice_invocationTimeout(timeout: 1500)

        locator = try locator!.ice_router(router: uncheckedCast(prx: router!, type: Ice.RouterPrx.self))
        b1 = try b1!.ice_locator(locator: uncheckedCast(prx: locator!, type: Ice.LocatorPrx.self))

        let proxyProps = try communicator.proxyToProperty(proxy: b1!, property: "Test")
        try test(proxyProps.count == 21)

        try test(proxyProps["Test"] == "test -t -e 1.0")
        try test(proxyProps["Test.CollocationOptimized"] == "1")
        try test(proxyProps["Test.ConnectionCached"] == "1")
        try test(proxyProps["Test.PreferSecure"] == "0")
        try test(proxyProps["Test.EndpointSelection"] == "Ordered")
        try test(proxyProps["Test.LocatorCacheTimeout"] == "100")
        try test(proxyProps["Test.InvocationTimeout"] == "1234")

        try test(proxyProps["Test.Locator"] ==
            "locator -t -e " + Ice.encodingVersionToString(encoding: currentEncoding))
        // Locator collocation optimization is always disabled.
        //test(proxyProps["Test.Locator.CollocationOptimized"].Equals("1"));
        try test(proxyProps["Test.Locator.ConnectionCached"] == "0")
        try test(proxyProps["Test.Locator.PreferSecure"] == "1")
        try test(proxyProps["Test.Locator.EndpointSelection"] == "Random")
        try test(proxyProps["Test.Locator.LocatorCacheTimeout"] == "300")
        try test(proxyProps["Test.Locator.InvocationTimeout"] == "1500")

        try test(proxyProps["Test.Locator.Router"] ==
            "router -t -e " + Ice.encodingVersionToString(encoding: Ice.currentEncoding))
        try test(proxyProps["Test.Locator.Router.CollocationOptimized"] == "0")
        try test(proxyProps["Test.Locator.Router.ConnectionCached"] == "1")
        try test(proxyProps["Test.Locator.Router.PreferSecure"] == "1")
        try test(proxyProps["Test.Locator.Router.EndpointSelection"] == "Random")
        try test(proxyProps["Test.Locator.Router.LocatorCacheTimeout"] == "200")
        try test(proxyProps["Test.Locator.Router.InvocationTimeout"] == "1500")

        writer.writeLine("ok")

        writer.write("testing ice_getCommunicator... ")
        try test(baseProxy!.ice_getCommunicator() === communicator)
        writer.writeLine("ok")

        writer.write("testing proxy methods... ")
        try test(baseProxy!.ice_facet(facet: "facet").ice_getFacet() == "facet")
        try test(baseProxy!.ice_adapterId(id: "id").ice_getAdapterId() == "id")
        try test(baseProxy!.ice_twoway().ice_isTwoway())
        try test(baseProxy!.ice_oneway().ice_isOneway())
        try test(baseProxy!.ice_batchOneway().ice_isBatchOneway())
        try test(baseProxy!.ice_datagram().ice_isDatagram())
        try test(baseProxy!.ice_batchDatagram().ice_isBatchDatagram())
        try test(baseProxy!.ice_secure(secure: true).ice_isSecure())
        try test(!baseProxy!.ice_secure(secure: false).ice_isSecure())
        try test(baseProxy!.ice_collocationOptimized(collocated: true)!.ice_isCollocationOptimized())
        try test(!baseProxy!.ice_collocationOptimized(collocated: false)!.ice_isCollocationOptimized())
        try test(baseProxy!.ice_preferSecure(preferSecure: true).ice_isPreferSecure())
        try test(!baseProxy!.ice_preferSecure(preferSecure: false).ice_isPreferSecure())

        do {
            _ = try baseProxy!.ice_timeout(timeout: 0)
            try test(false)
        } catch {
            // TODO handle argument_exception
        }

        do {
            _ = try baseProxy!.ice_timeout(timeout: -1)
        } catch {
            // TODO handle argument_exception
            try test(false)
        }

        do {
            _ = try baseProxy!.ice_timeout(timeout: -2)
            try test(false)
        } catch {
            // TODO handle argument_exception
        }

        do {
            _ = try baseProxy!.ice_invocationTimeout(timeout: 0)
            try test(false)
        } catch {
            // TODO handle argument_exception
        }

        do {
            _ = try baseProxy!.ice_invocationTimeout(timeout: -1)
            _ = try baseProxy!.ice_invocationTimeout(timeout: -2)
        } catch {
            // TODO handle argument_exception
            try test(false)
        }

        do {
            _ = try baseProxy!.ice_invocationTimeout(timeout: -3)
            try test(false)
        } catch {
            // TODO handle argument_exception
        }

        do {
            _ = try baseProxy!.ice_locatorCacheTimeout(timeout: 0)
        } catch {
            // TODO handle argument_exception
            try test(false)
        }

        do {
            _ = try baseProxy!.ice_locatorCacheTimeout(timeout: -1)
        } catch {
            // TODO handle argument_exception
            try test(false)
        }

        do {
            _ = try baseProxy!.ice_locatorCacheTimeout(timeout: -2)
            try test(false)
        } catch {
            // TODO handle argument_exception
        }

        writer.writeLine("ok")

        writer.write("testing proxy comparison... ")

        try test(communicator.stringToProxy(str: "foo") == communicator.stringToProxy(str: "foo"))
        try test(communicator.stringToProxy(str: "foo") != communicator.stringToProxy(str: "foo2"))

        var compObj = try communicator.stringToProxy(str: "foo")

        try test(compObj!.ice_facet(facet: "facet") == compObj!.ice_facet(facet: "facet"))
        try test(compObj!.ice_facet(facet: "facet") != compObj!.ice_facet(facet: "facet1"))

        try test(compObj!.ice_oneway() == compObj!.ice_oneway())
        try test(compObj!.ice_oneway() != compObj!.ice_twoway())

        try test(compObj!.ice_secure(secure: true) == compObj!.ice_secure(secure: true))
        try test(compObj!.ice_secure(secure: false) != compObj!.ice_secure(secure: true))

        try test(compObj!.ice_collocationOptimized(collocated: true) == compObj!.ice_collocationOptimized(collocated: true))
        try test(compObj!.ice_collocationOptimized(collocated: false) != compObj!.ice_collocationOptimized(collocated: true))

        try test(compObj!.ice_connectionCached(cached: true) == compObj!.ice_connectionCached(cached: true))
        try test(compObj!.ice_connectionCached(cached: false) != compObj!.ice_connectionCached(cached: true))

        try test(compObj!.ice_endpointSelection(type: Ice.EndpointSelectionType.Random) ==
                 compObj!.ice_endpointSelection(type: Ice.EndpointSelectionType.Random))
        try test(compObj!.ice_endpointSelection(type: Ice.EndpointSelectionType.Random) !=
                 compObj!.ice_endpointSelection(type: Ice.EndpointSelectionType.Ordered))

        try test(compObj!.ice_connectionId(id: "id2") == compObj!.ice_connectionId(id: "id2"))
        try test(compObj!.ice_connectionId(id: "id1") != compObj!.ice_connectionId(id: "id2"))
        try test(compObj!.ice_connectionId(id: "id1").ice_getConnectionId() == "id1")
        try test(compObj!.ice_connectionId(id: "id2").ice_getConnectionId() == "id2")

        try test(compObj!.ice_compress(compress: true) == compObj!.ice_compress(compress: true))
        try test(compObj!.ice_compress(compress: false) != compObj!.ice_compress(compress: true))

        try test(compObj!.ice_getCompress() == nil)
        try test(compObj!.ice_compress(compress: true).ice_getCompress() == true)
        try test(compObj!.ice_compress(compress: false).ice_getCompress() == false)

        try test(compObj!.ice_timeout(timeout: 20) == compObj!.ice_timeout(timeout: 20))
        try test(compObj!.ice_timeout(timeout: 10) != compObj!.ice_timeout(timeout: 20))

        try test(compObj!.ice_getTimeout() == nil)
        try test(compObj!.ice_timeout(timeout: 10).ice_getTimeout() == 10)
        try test(compObj!.ice_timeout(timeout: 20).ice_getTimeout() == 20)

        let loc1 = uncheckedCast(prx: try communicator.stringToProxy(str: "loc1:default -p 10000")!,
                                 type: Ice.LocatorPrx.self)
        let loc2 = uncheckedCast(prx: try communicator.stringToProxy(str: "loc2:default -p 10000")!,
                                 type: Ice.LocatorPrx.self)

        try test(compObj!.ice_locator(locator: nil) == compObj!.ice_locator(locator: nil))
        try test(compObj!.ice_locator(locator: loc1) == compObj!.ice_locator(locator: loc1))
        try test(compObj!.ice_locator(locator: loc1) != compObj!.ice_locator(locator: nil))
        try test(compObj!.ice_locator(locator: nil) != compObj!.ice_locator(locator: loc2))
        try test(compObj!.ice_locator(locator: loc1) != compObj!.ice_locator(locator: loc2))

        let rtr1 = uncheckedCast(prx: try communicator.stringToProxy(str: "rtr1:default -p 10000")!,
                                 type: Ice.RouterPrx.self)
        let rtr2 = uncheckedCast(prx: try communicator.stringToProxy(str: "rtr2:default -p 10000")!,
                                 type: Ice.RouterPrx.self)

        try test(compObj!.ice_router(router: nil) == compObj!.ice_router(router: nil))
        try test(compObj!.ice_router(router: rtr1) == compObj!.ice_router(router: rtr1))
        try test(compObj!.ice_router(router: rtr1) != compObj!.ice_router(router: nil))
        try test(compObj!.ice_router(router: nil) != compObj!.ice_router(router: rtr2))
        try test(compObj!.ice_router(router: rtr1) != compObj!.ice_router(router: rtr2))

        let ctx1 = ["ctx1": "v1"]
        let ctx2 = ["ctx2": "v2"]

        try test(compObj!.ice_context(context: Ice.Context()) == compObj!.ice_context(context: Ice.Context()))
        try test(compObj!.ice_context(context: ctx1) == compObj!.ice_context(context: ctx1))
        try test(compObj!.ice_context(context: ctx1) != compObj!.ice_context(context: Ice.Context()))
        try test(compObj!.ice_context(context: Ice.Context()) != compObj!.ice_context(context: ctx2))
        try test(compObj!.ice_context(context: ctx1) != compObj!.ice_context(context: ctx2))

        try test(compObj!.ice_preferSecure(preferSecure: true) == compObj!.ice_preferSecure(preferSecure: true))
        try test(compObj!.ice_preferSecure(preferSecure: true) != compObj!.ice_preferSecure(preferSecure: false))

        var compObj1 = try communicator.stringToProxy(str: "foo:tcp -h 127.0.0.1 -p 10000")
        var compObj2 = try communicator.stringToProxy(str: "foo:tcp -h 127.0.0.1 -p 10001")
        try test(compObj1 != compObj2)

        compObj1 = try communicator.stringToProxy(str: "foo@MyAdapter1")
        compObj2 = try communicator.stringToProxy(str: "foo@MyAdapter2")
        try test(compObj1 != compObj2)

        try test(compObj1!.ice_locatorCacheTimeout(timeout: 20) == compObj1!.ice_locatorCacheTimeout(timeout: 20))
        try test(compObj1!.ice_locatorCacheTimeout(timeout: 10) != compObj1!.ice_locatorCacheTimeout(timeout: 20))

        try test(compObj1!.ice_invocationTimeout(timeout: 20) == compObj1!.ice_invocationTimeout(timeout: 20))
        try test(compObj1!.ice_invocationTimeout(timeout: 10) != compObj1!.ice_invocationTimeout(timeout: 20))

        compObj1 = try communicator.stringToProxy(str: "foo:tcp -h 127.0.0.1 -p 1000")
        compObj2 = try communicator.stringToProxy(str: "foo@MyAdapter1")
        try test(compObj1 != compObj2)

        let endpts1 = try communicator.stringToProxy(str: "foo:tcp -h 127.0.0.1 -p 10000")!.ice_getEndpoints()
        let endpts2 = try communicator.stringToProxy(str: "foo:tcp -h 127.0.0.1 -p 10001")!.ice_getEndpoints()

        try test(endpts1[0] != endpts2[0])
        try test(endpts1[0] == communicator.stringToProxy(str: "foo:tcp -h 127.0.0.1 -p 10000")!.ice_getEndpoints()[0])

        let baseConnection = try baseProxy!.ice_getConnection()
        if baseConnection != nil {
            let baseConnection2 = try baseProxy!.ice_connectionId(id: "base2").ice_getConnection()
            compObj1 = try compObj1!.ice_fixed(connection: baseConnection!)
            compObj2 = try compObj2!.ice_fixed(connection: baseConnection2!)
            try test(compObj1 != compObj2)
        }
        writer.writeLine("ok")

        writer.write("testing checked cast... ")
        var cl = try checkedCast(prx: baseProxy!, type: MyClassPrx.self)
        try test(cl != nil)
        let derived = try checkedCast(prx: cl!, type: MyDerivedClassPrx.self)
        try test(derived != nil)
        try test(cl == baseProxy)
        try test(derived == baseProxy)
        try test(cl == derived)
        writer.writeLine("ok")

        writer.write("testing checked cast with context... ")
        var c = try cl!.getContext()
        try test(c.isEmpty)
        c = ["one": "hello", "two": "world"]
        cl = try checkedCast(prx: baseProxy!, type: MyClassPrx.self, context: c)
        let c2 = try cl!.getContext()
        try test(c == c2)
        writer.writeLine("ok")

        writer.write("testing ice_fixed... ")
        do {
            let connection = try cl!.ice_getConnection()
            if connection != nil {
                let prx = try cl!.ice_fixed(connection: connection!)
                try prx!.ice_ping()
                try test(cl!.ice_secure(secure: true).ice_fixed(connection: connection!)!.ice_isSecure())
                try test(cl!.ice_facet(facet: "facet").ice_fixed(connection: connection!)!.ice_getFacet() == "facet")
                try test(cl!.ice_oneway().ice_fixed(connection: connection!)!.ice_isOneway())
                let ctx = ["one": "hello", "two": "world"]
                try test(cl!.ice_fixed(connection: connection!)!.ice_getContext().isEmpty)
                try test(cl!.ice_context(context: ctx).ice_fixed(connection: connection!)!.ice_getContext().count == 2)
                try test(cl!.ice_fixed(connection: connection!)!.ice_getInvocationTimeout() == -1)
                try test(cl!.ice_invocationTimeout(timeout: 10).ice_fixed(connection: connection!)!.ice_getInvocationTimeout() == 10)
                try test(cl!.ice_fixed(connection: connection!)!.ice_getConnection() === connection)
                try test(cl!.ice_fixed(connection: connection!)!.ice_fixed(connection: connection!)!.ice_getConnection() === connection)
                try test(cl!.ice_fixed(connection: connection!)!.ice_getTimeout() == nil)
                try test(cl!.ice_compress(compress: true).ice_fixed(connection: connection!)!.ice_getCompress()!)
                let fixedConnection = try cl!.ice_connectionId(id: "ice_fixed").ice_getConnection()
                try test(cl!.ice_fixed(connection: connection!)!.ice_fixed(connection: fixedConnection!)!.ice_getConnection() ===
                         fixedConnection)
                do {
                    try cl!.ice_secure(secure: !connection!.getEndpoint().getInfo()!.secure()).ice_fixed(connection: connection!)!.ice_ping()
                } catch is Ice.NoEndpointException {}

                do {
                    try cl!.ice_datagram().ice_fixed(connection: connection!)!.ice_ping()
                } catch is Ice.NoEndpointException {}
            } else {
                do {
                    _ = try cl!.ice_fixed(connection: connection!)
                    try test(false)
                } catch {
                    // Expected with null connection.
                }
            }
        }
        writer.writeLine("ok")

        writer.write("testing encoding versioning... ")
        var ref20 = "test -e 2.0:\(self.getTestEndpoint(num: 0))"
        var cl20 = try uncheckedCast(prx: communicator.stringToProxy(str: ref20)!, type: MyClassPrx.self)!
        do {
            try cl20.ice_ping()
            try test(false)
        } catch is Ice.UnsupportedEncodingException {
            // Server 2.0 endpoint doesn't support 1.1 version.
        }

        var ref10 = "test -e 1.0:\(self.getTestEndpoint(num: 0))"
        var cl10 = try uncheckedCast(prx: communicator.stringToProxy(str: ref10)!, type: MyClassPrx.self)!
        try cl10.ice_ping()
        try cl10.ice_encodingVersion(encoding: Ice.Encoding_1_0).ice_ping()
        try cl!.ice_encodingVersion(encoding: Ice.Encoding_1_0).ice_ping()

        // 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
        // call will use the 1.1 encoding
        var ref13 = "test -e 1.3:\(getTestEndpoint(num: 0))"
        var cl13 = try uncheckedCast(prx: communicator.stringToProxy(str: ref13)!, type: MyClassPrx.self)!
        try cl13.ice_ping()

        do {
            // Send request with bogus 1.2 encoding.
            let version = Ice.EncodingVersion(major: 1, minor: 2)
            let os = Ice.OutputStream(communicator: communicator)
            os.startEncapsulation()
            os.endEncapsulation()
            var inEncaps = os.finished()
            inEncaps[4] = version.major
            inEncaps[5] = version.minor

            _ = try cl!.ice_invoke(operation: "ice_ping", mode: Ice.OperationMode.Normal, inEncaps: inEncaps)
            try test(false)
        } catch let ex as Ice.UnknownLocalException {
            // The server thrown an UnsupportedEncodingException
            try test(ex.unknown.contains("UnsupportedEncodingException"))
        }

        do {
            // Send request with bogus 2.0 encoding.
            let version = Ice.EncodingVersion(major: 2, minor: 0)
            let os = Ice.OutputStream(communicator: communicator)
            os.startEncapsulation()
            os.endEncapsulation()
            var inEncaps = os.finished()
            inEncaps[4] = version.major
            inEncaps[5] = version.minor
            _ = try cl!.ice_invoke(operation: "ice_ping", mode: Ice.OperationMode.Normal, inEncaps: inEncaps)
            try test(false)
        } catch let ex as Ice.UnknownLocalException {
            // The server thrown an UnsupportedEncodingException
            try test(ex.unknown.contains("UnsupportedEncodingException"))
        }
        writer.writeLine("ok")

        writer.write("testing protocol versioning... ")
        ref20 = "test -p 2.0:\(self.getTestEndpoint(num: 0))"
        cl20 = try uncheckedCast(prx: communicator.stringToProxy(str: ref20)!, type: MyClassPrx.self)!
        do {
            try cl20.ice_ping()
            try test(false)
        } catch is Ice.UnsupportedProtocolException {
            // Server 2.0 proxy doesn't support 1.0 version.
        }

        ref10 = "test -p 1.0:\(self.getTestEndpoint(num: 0))"
        cl10 = try uncheckedCast(prx: communicator.stringToProxy(str: ref10)!, type: MyClassPrx.self)!
        try cl10.ice_ping();
        
        // 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
        // call will use the 1.1 protocol
        ref13 = "test -p 1.3:\(self.getTestEndpoint(num: 0))"
        cl13 = try uncheckedCast(prx: communicator.stringToProxy(str: ref13)!, type: MyClassPrx.self)!
        try cl13.ice_ping()
        writer.writeLine("ok")
    }
}
