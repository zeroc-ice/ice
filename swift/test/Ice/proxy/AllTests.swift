//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

public func allTests(helper: TestHelper) throws -> MyClassPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let writer = helper.getWriter()
    writer.write("testing stringToProxy... ")

    let communicator = helper.communicator()
    let rf = "test:\(helper.getTestEndpoint(num: 0))"
    let baseProxy = try communicator.stringToProxy(rf)!
    var b1 = try communicator.stringToProxy("test")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getAdapterId().isEmpty &&
        b1.ice_getFacet().isEmpty)
    b1 = try communicator.stringToProxy("test ")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getAdapterId().isEmpty &&
        b1.ice_getFacet().isEmpty)
    b1 = try communicator.stringToProxy(" test ")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getAdapterId().isEmpty &&
        b1.ice_getFacet().isEmpty)
    b1 = try communicator.stringToProxy(" test")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getAdapterId().isEmpty &&
        b1.ice_getFacet().isEmpty)
    b1 = try communicator.stringToProxy("'test -f facet'")!
    try test(b1.ice_getIdentity().name == "test -f facet" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getFacet().isEmpty)

    do {
        _ = try communicator.stringToProxy("\"test -f facet'")
        try test(false)
    } catch is Ice.ProxyParseException {}

    b1 = try communicator.stringToProxy("\"test -f facet\"")!
    try test(b1.ice_getIdentity().name == "test -f facet" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getFacet().isEmpty)

    b1 = try communicator.stringToProxy("\"test -f facet@test\"")!
    try test(b1.ice_getIdentity().name == "test -f facet@test" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getFacet().isEmpty)

    b1 = try communicator.stringToProxy("\"test -f facet@test @test\"")!
    try test(b1.ice_getIdentity().name == "test -f facet@test @test" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getFacet().isEmpty)

    do {
        _ = try communicator.stringToProxy("test test")
        try test(false)
    } catch is Ice.ProxyParseException {}

    b1 = try communicator.stringToProxy("test\\040test")!
    try test(b1.ice_getIdentity().name == "test test" &&
        b1.ice_getIdentity().category.isEmpty)
    do {
        _ = try communicator.stringToProxy("test\\777")
        try test(false)
    } catch is Ice.IdentityParseException {}

    b1 = try communicator.stringToProxy("test\\40test")!
    try test(b1.ice_getIdentity().name == "test test")

    // Test some octal and hex corner cases.
    b1 = try communicator.stringToProxy("test\\4test")!
    try test(b1.ice_getIdentity().name == "test\u{0004}test")
    b1 = try communicator.stringToProxy("test\\04test")!
    try test(b1.ice_getIdentity().name == "test\u{0004}test")
    b1 = try communicator.stringToProxy("test\\004test")!
    try test(b1.ice_getIdentity().name == "test\u{0004}test")
    b1 = try communicator.stringToProxy("test\\1114test")!
    try test(b1.ice_getIdentity().name == "test\u{0049}4test")

    b1 = try communicator.stringToProxy("test\\b\\f\\n\\r\\t\\'\\\"\\\\test")!
    try test(b1.ice_getIdentity().name == "test\u{0008}\u{000C}\n\r\t\'\"\\test" &&
        b1.ice_getIdentity().category.isEmpty)

    b1 = try communicator.stringToProxy("category/test")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category == "category" &&
        b1.ice_getAdapterId().isEmpty)

    try test(communicator.stringToProxy("") == nil)
    try test(communicator.stringToProxy("\"\"") == nil)

    do {
        _ = try communicator.stringToProxy("\"\" test") // Invalid trailing characters.
        try test(false)
    } catch is Ice.ProxyParseException {}

    do {
        _ = try communicator.stringToProxy("test:") // Missing endpoint.
        try test(false)
    } catch is Ice.EndpointParseException {}

    b1 = try communicator.stringToProxy("test@adapter")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getAdapterId() == "adapter")

    do {
        _ = try communicator.stringToProxy("id@adapter test")
        try test(false)
    } catch is Ice.ProxyParseException {}

    b1 = try communicator.stringToProxy("category/test@adapter")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category == "category" &&
        b1.ice_getAdapterId() == "adapter")
    b1 = try communicator.stringToProxy("category/test@adapter:tcp")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category == "category" &&
        b1.ice_getAdapterId() == "adapter:tcp")
    b1 = try communicator.stringToProxy("'category 1/test'@adapter")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category == "category 1" &&
        b1.ice_getAdapterId() == "adapter")
    b1 = try communicator.stringToProxy("'category/test 1'@adapter")!
    try test(b1.ice_getIdentity().name == "test 1" &&
        b1.ice_getIdentity().category == "category" &&
        b1.ice_getAdapterId() == "adapter")
    b1 = try communicator.stringToProxy("'category/test'@'adapter 1'")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category == "category" &&
        b1.ice_getAdapterId() == "adapter 1")
    b1 = try communicator.stringToProxy("\"category \\/test@foo/test\"@adapter")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category == "category /test@foo" &&
        b1.ice_getAdapterId() == "adapter")
    b1 = try communicator.stringToProxy("\"category \\/test@foo/test\"@\"adapter:tcp\"")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category == "category /test@foo" &&
        b1.ice_getAdapterId() == "adapter:tcp")

    b1 = try communicator.stringToProxy("id -f facet")!
    try test(b1.ice_getIdentity().name == "id" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getFacet() == "facet")
    b1 = try communicator.stringToProxy("id -f 'facet x'")!
    try test(b1.ice_getIdentity().name == "id" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getFacet() == "facet x")
    b1 = try communicator.stringToProxy("id -f \"facet x\"")!
    try test(b1.ice_getIdentity().name == "id" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getFacet() == "facet x")

    do {
        b1 = try communicator.stringToProxy("id -f \"facet x")!
        try test(false)
    } catch is Ice.ProxyParseException {}

    do {
        b1 = try communicator.stringToProxy("id -f \'facet x")!
        try test(false)
    } catch is Ice.ProxyParseException {}

    b1 = try communicator.stringToProxy("test -f facet:tcp")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getFacet() == "facet" &&
        b1.ice_getAdapterId().isEmpty)
    b1 = try communicator.stringToProxy("test -f \"facet:tcp\"")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getFacet() == "facet:tcp" &&
        b1.ice_getAdapterId().isEmpty)
    b1 = try communicator.stringToProxy("test -f facet@test")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getFacet() == "facet" &&
        b1.ice_getAdapterId() == "test")
    b1 = try communicator.stringToProxy("test -f 'facet@test'")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getFacet() == "facet@test" &&
        b1.ice_getAdapterId().isEmpty)
    b1 = try communicator.stringToProxy("test -f 'facet@test'@test")!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getFacet() == "facet@test" &&
        b1.ice_getAdapterId() == "test")

    do {
        b1 = try communicator.stringToProxy("test -f facet@test @test")!
        try test(false)
    } catch is Ice.ProxyParseException {}
    b1 = try communicator.stringToProxy("test")!
    try test(b1.ice_isTwoway())
    b1 = try communicator.stringToProxy("test -t")!
    try test(b1.ice_isTwoway())
    b1 = try communicator.stringToProxy("test -o")!
    try test(b1.ice_isOneway())
    b1 = try communicator.stringToProxy("test -O")!
    try test(b1.ice_isBatchOneway())
    b1 = try communicator.stringToProxy("test -d")!
    try test(b1.ice_isDatagram())
    b1 = try communicator.stringToProxy("test -D")!
    try test(b1.ice_isBatchDatagram())
    b1 = try communicator.stringToProxy("test")!
    try test(!b1.ice_isSecure())
    b1 = try communicator.stringToProxy("test -s")!
    try test(b1.ice_isSecure())

    try test(b1.ice_getEncodingVersion() == Ice.currentEncoding)

    b1 = try communicator.stringToProxy("test -e 1.0")!
    try test(b1.ice_getEncodingVersion().major == 1 &&
        b1.ice_getEncodingVersion().minor == 0)

    b1 = try communicator.stringToProxy("test -e 6.5")!
    try test(b1.ice_getEncodingVersion().major == 6 &&
        b1.ice_getEncodingVersion().minor == 5)

    b1 = try communicator.stringToProxy("test -p 1.0 -e 1.0")!
    try test(b1.ice_toString() == "test -t -e 1.0")

    b1 = try communicator.stringToProxy("test -p 6.5 -e 1.0")!
    try test(b1.ice_toString() == "test -t -p 6.5 -e 1.0")

    do {
        _ = try communicator.stringToProxy("test:tcp@adapterId")
        try test(false)
    } catch is Ice.EndpointParseException {}

    do {
        _ = try communicator.stringToProxy("test: :tcp")
        try test(false)
    } catch is Ice.EndpointParseException {}

    //
    // Test invalid endpoint syntax
    //
    do {
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
    } catch is Ice.EndpointParseException {}

    //
    // Test for bug ICE-5543: escaped escapes in stringToIdentity
    //
    var id = Ice.Identity(name: "test", category: ",X2QNUAzSBcJ_e$AV;E\\")
    var id2 = try Ice.stringToIdentity(communicator.identityToString(id))
    try test(id == id2)

    id = Ice.Identity(name: "test", category: ",X2QNUAz\\SB\\/cJ_e$AV;E\\\\")
    id2 = try Ice.stringToIdentity(communicator.identityToString(id))
    try test(id == id2)

    id = Ice.Identity(name: "/test", category: "cat/")
    var idStr = try communicator.identityToString(id)
    try test(idStr == "cat\\//\\/test")
    id2 = try Ice.stringToIdentity(idStr)
    try test(id == id2)

    // Input string with various pitfalls
    id = try Ice.stringToIdentity("\\342\\x82\\254\\60\\x9\\60\\")
    try test(id.name == "€0\t0\\" && id.category == "")

    do {
        // Illegal character < 32
        id = try Ice.stringToIdentity("xx\u{01}FooBar")
        try test(false)
    } catch is Ice.IdentityParseException {}

    do {
        // Illegal surrogate
        id = try Ice.stringToIdentity("xx\\ud911")
        try test(false)
    } catch is Ice.IdentityParseException {}

    // Testing bytes 127(\x7F, \177) and €
    id = Ice.Identity(name: "test", category: "\u{007f}€")

    idStr = try Ice.identityToString(id: id, mode: Ice.ToStringMode.Unicode)
    try test(idStr == "\\u007f€/test")
    id2 = try Ice.stringToIdentity(idStr)
    try test(id == id2)
    try test(Ice.identityToString(id: id) == idStr)

    idStr = try Ice.identityToString(id: id, mode: Ice.ToStringMode.ASCII)
    try test(idStr == "\\u007f\\u20ac/test")
    id2 = try Ice.stringToIdentity(idStr)
    try test(id == id2)

    idStr = try Ice.identityToString(id: id, mode: Ice.ToStringMode.Compat)
    try test(idStr == "\\177\\342\\202\\254/test")
    id2 = try Ice.stringToIdentity(idStr)
    try test(id == id2)

    id2 = try Ice.stringToIdentity(communicator.identityToString(id))
    try test(id == id2)

    // More unicode character
    id = Ice.Identity(name: "banana \u{000E}-\u{1f34c}\u{20ac}\u{00a2}\u{0024}",
                      category: "greek \u{1016a}")

    idStr = try Ice.identityToString(id: id, mode: Ice.ToStringMode.Unicode)
    try test(idStr == "greek \u{1016a}/banana \\u000e-\u{1f34c}\u{20ac}\u{00a2}$")
    id2 = try Ice.stringToIdentity(idStr)
    try test(id == id2)

    idStr = try Ice.identityToString(id: id, mode: Ice.ToStringMode.ASCII)
    try test(idStr == "greek \\U0001016a/banana \\u000e-\\U0001f34c\\u20ac\\u00a2$")
    id2 = try Ice.stringToIdentity(idStr)
    try test(id == id2)

    idStr = try Ice.identityToString(id: id, mode: Ice.ToStringMode.Compat)
    id2 = try Ice.stringToIdentity(idStr)
    try test(idStr == "greek \\360\\220\\205\\252/banana \\016-\\360\\237\\215\\214\\342\\202\\254\\302\\242$")
    try test(id == id2)
    writer.writeLine("ok")

    writer.write("testing proxyToString... ")
    b1 = try communicator.stringToProxy(rf)!
    var b2 = try communicator.stringToProxy(communicator.proxyToString(b1))!

    try test(b1 == b2)

    if(try b1.ice_getConnection() != nil) // not colloc-optimized target
    {
        b2 = try b1.ice_getConnection()!.createProxy(Ice.stringToIdentity("fixed"))!
        let str = try communicator.proxyToString(b2)
        try test(b2.ice_toString() == str)
        let str2 = try b1.ice_identity(b2.ice_getIdentity()).ice_secure(b2.ice_isSecure()).ice_toString()
        // Verify that the stringified fixed proxy is the same as a regular stringified proxy
        // but without endpoints
        try test(str2.hasPrefix("\(str):"))
    }
    writer.writeLine("ok")

    writer.write("testing propertyToProxy... ")
    let prop = communicator.getProperties()
    let propertyPrefix = "Foo.Proxy"
    try prop.setProperty(key: propertyPrefix, value: "test:\(helper.getTestEndpoint(num: 0))")
    b1 = try communicator.propertyToProxy(propertyPrefix)!
    try test(b1.ice_getIdentity().name == "test" &&
        b1.ice_getIdentity().category.isEmpty &&
        b1.ice_getAdapterId().isEmpty &&
        b1.ice_getFacet().isEmpty)

    var property = "\(propertyPrefix).Locator"
    try test(b1.ice_getLocator() == nil)
    try prop.setProperty(key: property, value: "locator:default -p 10000")
    b1 = try communicator.propertyToProxy(propertyPrefix)!

    try test(b1.ice_getLocator() != nil &&
        b1.ice_getLocator()!.ice_getIdentity().name == "locator")
    try prop.setProperty(key: property, value: "")

    property = "\(propertyPrefix).LocatorCacheTimeout"
    try test(b1.ice_getLocatorCacheTimeout() == -1)
    try prop.setProperty(key: property, value: "1")
    b1 = try communicator.propertyToProxy(propertyPrefix)!
    try test(b1.ice_getLocatorCacheTimeout() == 1)
    try prop.setProperty(key: property, value: "")

    // Now retest with an indirect proxy.
    try prop.setProperty(key: propertyPrefix, value: "test")
    property = "\(propertyPrefix).Locator"
    try prop.setProperty(key: property, value: "locator:default -p 10000")
    b1 = try communicator.propertyToProxy(propertyPrefix)!
    try test(b1.ice_getLocator() != nil &&
        b1.ice_getLocator()!.ice_getIdentity().name == "locator")
    try prop.setProperty(key: property, value: "")

    property = "\(propertyPrefix).LocatorCacheTimeout"
    try test(b1.ice_getLocatorCacheTimeout() == -1)
    try prop.setProperty(key: property, value: "1")
    b1 = try communicator.propertyToProxy(propertyPrefix)!
    try test(b1.ice_getLocatorCacheTimeout() == 1)
    try prop.setProperty(key: property, value: "")

    // This cannot be tested so easily because the property is cached
    // on communicator initialization.
    //
    //prop.setProperty("Ice.Default.LocatorCacheTimeout", "60");
    //b1 = communicator.propertyToProxy(propertyPrefix);
    //test(b1.ice_getLocatorCacheTimeout() == 60);
    //prop.setProperty("Ice.Default.LocatorCacheTimeout", "");

    try prop.setProperty(key: propertyPrefix, value: "test:\(helper.getTestEndpoint(num: 0))")

    property = "\(propertyPrefix).Router"
    try test(b1.ice_getRouter() == nil)
    try prop.setProperty(key: property, value: "router:default -p 10000")
    b1 = try communicator.propertyToProxy(propertyPrefix)!
    try test(b1.ice_getRouter() != nil &&
        b1.ice_getRouter()!.ice_getIdentity().name == "router")
    try prop.setProperty(key: property, value: "")

    property = "\(propertyPrefix).PreferSecure"
    try test(!b1.ice_isPreferSecure())
    try prop.setProperty(key: property, value: "1")
    b1 = try communicator.propertyToProxy(propertyPrefix)!
    try test(b1.ice_isPreferSecure())
    try prop.setProperty(key: property, value: "")

    property = "\(propertyPrefix).ConnectionCached"
    try test(b1.ice_isConnectionCached())
    try prop.setProperty(key: property, value: "0")
    b1 = try communicator.propertyToProxy(propertyPrefix)!
    try test(!b1.ice_isConnectionCached())
    try prop.setProperty(key: property, value: "")

    property = "\(propertyPrefix).InvocationTimeout"
    try test(b1.ice_getInvocationTimeout() == -1)
    try prop.setProperty(key: property, value: "1000")
    b1 = try communicator.propertyToProxy(propertyPrefix)!
    try test(b1.ice_getInvocationTimeout() == 1000)
    try prop.setProperty(key: property, value: "")

    property = "\(propertyPrefix).EndpointSelection"
    try test(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random)
    try prop.setProperty(key: property, value: "Random")
    b1 = try communicator.propertyToProxy(propertyPrefix)!
    try test(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random)
    try prop.setProperty(key: property, value: "Ordered")
    b1 = try communicator.propertyToProxy(propertyPrefix)!
    try test(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered)
    try prop.setProperty(key: property, value: "")

    property = "\(propertyPrefix).CollocationOptimized"
    try test(b1.ice_isCollocationOptimized())
    try prop.setProperty(key: property, value: "0")
    b1 = try communicator.propertyToProxy(propertyPrefix)!
    try test(!b1.ice_isCollocationOptimized())
    try prop.setProperty(key: property, value: "")

    property = "\(propertyPrefix).Context.c1"
    try test(b1.ice_getContext()["c1"] == nil)
    try prop.setProperty(key: property, value: "TEST")
    b1 = try communicator.propertyToProxy(propertyPrefix)!
    try test(b1.ice_getContext()["c1"] == "TEST")

    property = "\(propertyPrefix).Context.c2"
    try test(b1.ice_getContext()["c2"] == nil)
    try prop.setProperty(key: property, value: "TEST")
    b1 = try communicator.propertyToProxy(propertyPrefix)!
    try test(b1.ice_getContext()["c2"] == "TEST")

    try prop.setProperty(key: "\(propertyPrefix).Context.c1", value: "")
    try prop.setProperty(key: "\(propertyPrefix).Context.c2", value: "")

    writer.writeLine("ok")

    writer.write("testing proxyToProperty... ")

    b1 = try communicator.stringToProxy("test")!
    b1 = try b1.ice_collocationOptimized(true)!
    b1 = try b1.ice_connectionCached(true)
    b1 = try b1.ice_preferSecure(false)
    b1 = try b1.ice_endpointSelection(Ice.EndpointSelectionType.Ordered)
    b1 = try b1.ice_locatorCacheTimeout(100)
    b1 = try b1.ice_invocationTimeout(1234)
    b1 = b1.ice_encodingVersion(Ice.EncodingVersion(major: 1, minor: 0))

    var router = try communicator.stringToProxy("router")!
    router = try router.ice_collocationOptimized(false)!
    router = try router.ice_connectionCached(true)
    router = try router.ice_preferSecure(true)
    router = try router.ice_endpointSelection(Ice.EndpointSelectionType.Random)
    router = try router.ice_locatorCacheTimeout(200)
    router = try router.ice_invocationTimeout(1500)

    var locator = try communicator.stringToProxy("locator")!
    locator = try locator.ice_collocationOptimized(true)!
    locator = try locator.ice_connectionCached(false)
    locator = try locator.ice_preferSecure(true)
    locator = try locator.ice_endpointSelection(Ice.EndpointSelectionType.Random)
    locator = try locator.ice_locatorCacheTimeout(300)
    locator = try locator.ice_invocationTimeout(1500)

    locator = try locator.ice_router(uncheckedCast(prx: router, type: Ice.RouterPrx.self))
    b1 = try b1.ice_locator(uncheckedCast(prx: locator, type: Ice.LocatorPrx.self))

    let proxyProps = try communicator.proxyToProperty(proxy: b1, property: "Test")
    try test(proxyProps.count == 21)

    try test(proxyProps["Test"] == "test -t -e 1.0")
    try test(proxyProps["Test.CollocationOptimized"] == "1")
    try test(proxyProps["Test.ConnectionCached"] == "1")
    try test(proxyProps["Test.PreferSecure"] == "0")
    try test(proxyProps["Test.EndpointSelection"] == "Ordered")
    try test(proxyProps["Test.LocatorCacheTimeout"] == "100")
    try test(proxyProps["Test.InvocationTimeout"] == "1234")

    try test(proxyProps["Test.Locator"] ==
        "locator -t -e " + Ice.encodingVersionToString(currentEncoding))
    // Locator collocation optimization is always disabled.
    //test(proxyProps["Test.Locator.CollocationOptimized"].Equals("1"));
    try test(proxyProps["Test.Locator.ConnectionCached"] == "0")
    try test(proxyProps["Test.Locator.PreferSecure"] == "1")
    try test(proxyProps["Test.Locator.EndpointSelection"] == "Random")
    try test(proxyProps["Test.Locator.LocatorCacheTimeout"] == "300")
    try test(proxyProps["Test.Locator.InvocationTimeout"] == "1500")

    try test(proxyProps["Test.Locator.Router"] ==
        "router -t -e " + Ice.encodingVersionToString(Ice.currentEncoding))
    try test(proxyProps["Test.Locator.Router.CollocationOptimized"] == "0")
    try test(proxyProps["Test.Locator.Router.ConnectionCached"] == "1")
    try test(proxyProps["Test.Locator.Router.PreferSecure"] == "1")
    try test(proxyProps["Test.Locator.Router.EndpointSelection"] == "Random")
    try test(proxyProps["Test.Locator.Router.LocatorCacheTimeout"] == "200")
    try test(proxyProps["Test.Locator.Router.InvocationTimeout"] == "1500")

    writer.writeLine("ok")

    writer.write("testing ice_getCommunicator... ")
    try test(baseProxy.ice_getCommunicator() === communicator)
    writer.writeLine("ok")

    writer.write("testing proxy methods... ")
    try test(baseProxy.ice_facet("facet").ice_getFacet() == "facet")
    try test(baseProxy.ice_adapterId("id").ice_getAdapterId() == "id")
    try test(baseProxy.ice_twoway().ice_isTwoway())
    try test(baseProxy.ice_oneway().ice_isOneway())
    try test(baseProxy.ice_batchOneway().ice_isBatchOneway())
    try test(baseProxy.ice_datagram().ice_isDatagram())
    try test(baseProxy.ice_batchDatagram().ice_isBatchDatagram())
    try test(baseProxy.ice_secure(true).ice_isSecure())
    try test(!baseProxy.ice_secure(false).ice_isSecure())
    try test(baseProxy.ice_collocationOptimized(true)!.ice_isCollocationOptimized())
    try test(!baseProxy.ice_collocationOptimized(false)!.ice_isCollocationOptimized())
    try test(baseProxy.ice_preferSecure(true).ice_isPreferSecure())
    try test(!baseProxy.ice_preferSecure(false).ice_isPreferSecure())

    do {
        _ = try baseProxy.ice_timeout(0)
        try test(false)
    } catch is Ice.RuntimeError {}

    do {
        _ = try baseProxy.ice_timeout(-1)
    } catch {
        try test(false)
    }

    do {
        _ = try baseProxy.ice_timeout(-2)
        try test(false)
    } catch is Ice.RuntimeError {}

    do {
        _ = try baseProxy.ice_invocationTimeout(0)
        try test(false)
    } catch is Ice.RuntimeError {}

    do {
        _ = try baseProxy.ice_invocationTimeout(-1)
        _ = try baseProxy.ice_invocationTimeout(-2)
    } catch {
        try test(false)
    }

    do {
        _ = try baseProxy.ice_invocationTimeout(-3)
        try test(false)
    } catch is Ice.RuntimeError {}

    do {
        _ = try baseProxy.ice_locatorCacheTimeout(0)
    } catch {
        try test(false)
    }

    do {
        _ = try baseProxy.ice_locatorCacheTimeout(-1)
    } catch {
        try test(false)
    }

    do {
        _ = try baseProxy.ice_locatorCacheTimeout(-2)
        try test(false)
    } catch is Ice.RuntimeError {}

    writer.writeLine("ok")

    writer.write("testing proxy comparison... ")

    try test(communicator.stringToProxy("foo") == communicator.stringToProxy("foo"))
    try test(communicator.stringToProxy("foo") != communicator.stringToProxy("foo2"))

    let compObj = try communicator.stringToProxy("foo")

    try test(compObj!.ice_facet("facet") == compObj!.ice_facet("facet"))
    try test(compObj!.ice_facet("facet") != compObj!.ice_facet("facet1"))

    try test(compObj!.ice_oneway() == compObj!.ice_oneway())
    try test(compObj!.ice_oneway() != compObj!.ice_twoway())

    try test(compObj!.ice_secure(true) == compObj!.ice_secure(true))
    try test(compObj!.ice_secure(false) != compObj!.ice_secure(true))

    try test(compObj!.ice_collocationOptimized(true) == compObj!.ice_collocationOptimized(true))
    try test(compObj!.ice_collocationOptimized(false) != compObj!.ice_collocationOptimized(true))

    try test(compObj!.ice_connectionCached(true) == compObj!.ice_connectionCached(true))
    try test(compObj!.ice_connectionCached(false) != compObj!.ice_connectionCached(true))

    try test(compObj!.ice_endpointSelection(Ice.EndpointSelectionType.Random) ==
        compObj!.ice_endpointSelection(Ice.EndpointSelectionType.Random))
    try test(compObj!.ice_endpointSelection(Ice.EndpointSelectionType.Random) !=
        compObj!.ice_endpointSelection(Ice.EndpointSelectionType.Ordered))

    try test(compObj!.ice_connectionId("id2") == compObj!.ice_connectionId("id2"))
    try test(compObj!.ice_connectionId("id1") != compObj!.ice_connectionId("id2"))
    try test(compObj!.ice_connectionId("id1").ice_getConnectionId() == "id1")
    try test(compObj!.ice_connectionId("id2").ice_getConnectionId() == "id2")

    try test(compObj!.ice_compress(true) == compObj!.ice_compress(true))
    try test(compObj!.ice_compress(false) != compObj!.ice_compress(true))

    try test(compObj!.ice_getCompress() == nil)
    try test(compObj!.ice_compress(true).ice_getCompress() == true)
    try test(compObj!.ice_compress(false).ice_getCompress() == false)

    try test(compObj!.ice_timeout(20) == compObj!.ice_timeout(20))
    try test(compObj!.ice_timeout(10) != compObj!.ice_timeout(20))

    try test(compObj!.ice_getTimeout() == nil)
    try test(compObj!.ice_timeout(10).ice_getTimeout() == 10)
    try test(compObj!.ice_timeout(20).ice_getTimeout() == 20)

    let loc1 = uncheckedCast(prx: try communicator.stringToProxy("loc1:default -p 10000")!,
                             type: Ice.LocatorPrx.self)
    let loc2 = uncheckedCast(prx: try communicator.stringToProxy("loc2:default -p 10000")!,
                             type: Ice.LocatorPrx.self)

    try test(compObj!.ice_locator(nil) == compObj!.ice_locator(nil))
    try test(compObj!.ice_locator(loc1) == compObj!.ice_locator(loc1))
    try test(compObj!.ice_locator(loc1) != compObj!.ice_locator(nil))
    try test(compObj!.ice_locator(nil) != compObj!.ice_locator(loc2))
    try test(compObj!.ice_locator(loc1) != compObj!.ice_locator(loc2))

    let rtr1 = uncheckedCast(prx: try communicator.stringToProxy("rtr1:default -p 10000")!,
                             type: Ice.RouterPrx.self)
    let rtr2 = uncheckedCast(prx: try communicator.stringToProxy("rtr2:default -p 10000")!,
                             type: Ice.RouterPrx.self)

    try test(compObj!.ice_router(nil) == compObj!.ice_router(nil))
    try test(compObj!.ice_router(rtr1) == compObj!.ice_router(rtr1))
    try test(compObj!.ice_router(rtr1) != compObj!.ice_router(nil))
    try test(compObj!.ice_router(nil) != compObj!.ice_router(rtr2))
    try test(compObj!.ice_router(rtr1) != compObj!.ice_router(rtr2))

    let ctx1 = ["ctx1": "v1"]
    let ctx2 = ["ctx2": "v2"]

    try test(compObj!.ice_context(Ice.Context()) == compObj!.ice_context(Ice.Context()))
    try test(compObj!.ice_context(ctx1) == compObj!.ice_context(ctx1))
    try test(compObj!.ice_context(ctx1) != compObj!.ice_context(Ice.Context()))
    try test(compObj!.ice_context(Ice.Context()) != compObj!.ice_context(ctx2))
    try test(compObj!.ice_context(ctx1) != compObj!.ice_context(ctx2))

    try test(compObj!.ice_preferSecure(true) == compObj!.ice_preferSecure(true))
    try test(compObj!.ice_preferSecure(true) != compObj!.ice_preferSecure(false))

    var compObj1 = try communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000")
    var compObj2 = try communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10001")
    try test(compObj1 != compObj2)

    compObj1 = try communicator.stringToProxy("foo@MyAdapter1")
    compObj2 = try communicator.stringToProxy("foo@MyAdapter2")
    try test(compObj1 != compObj2)

    try test(compObj1!.ice_locatorCacheTimeout(20) == compObj1!.ice_locatorCacheTimeout(20))
    try test(compObj1!.ice_locatorCacheTimeout(10) != compObj1!.ice_locatorCacheTimeout(20))

    try test(compObj1!.ice_invocationTimeout(20) == compObj1!.ice_invocationTimeout(20))
    try test(compObj1!.ice_invocationTimeout(10) != compObj1!.ice_invocationTimeout(20))

    compObj1 = try communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 1000")
    compObj2 = try communicator.stringToProxy("foo@MyAdapter1")
    try test(compObj1 != compObj2)

    let endpts1 = try communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000")!.ice_getEndpoints()
    let endpts2 = try communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10001")!.ice_getEndpoints()

    try test(endpts1[0] != endpts2[0])
    try test(endpts1[0] == communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000")!.ice_getEndpoints()[0])

    let baseConnection = try baseProxy.ice_getConnection()
    if baseConnection != nil {
        let baseConnection2 = try baseProxy.ice_connectionId("base2").ice_getConnection()
        compObj1 = try compObj1!.ice_fixed(baseConnection!)
        compObj2 = try compObj2!.ice_fixed(baseConnection2!)
        try test(compObj1 != compObj2)
    }
    writer.writeLine("ok")

    writer.write("testing checked cast... ")
    var cl = try checkedCast(prx: baseProxy, type: MyClassPrx.self)!
    let derived = try checkedCast(prx: cl, type: MyDerivedClassPrx.self)!
    try test(cl == baseProxy)
    try test(derived == baseProxy)
    try test(cl == derived)
    writer.writeLine("ok")

    writer.write("testing checked cast with context... ")
    var c = try cl.getContext()
    try test(c.isEmpty)
    c = ["one": "hello", "two": "world"]
    cl = try checkedCast(prx: baseProxy, type: MyClassPrx.self, context: c)!
    let c2 = try cl.getContext()
    try test(c == c2)
    writer.writeLine("ok")

    writer.write("testing ice_fixed... ")
    do {
        let connection = try cl.ice_getConnection()
        if connection != nil {
            let prx = try cl.ice_fixed(connection!)!
            try prx.ice_ping()
            try test(cl.ice_secure(true).ice_fixed(connection!)!.ice_isSecure())
            try test(cl.ice_facet("facet").ice_fixed(connection!)!.ice_getFacet() == "facet")
            try test(cl.ice_oneway().ice_fixed(connection!)!.ice_isOneway())
            let ctx = ["one": "hello", "two": "world"]
            try test(cl.ice_fixed(connection!)!.ice_getContext().isEmpty)
            try test(cl.ice_context(ctx).ice_fixed(connection!)!.ice_getContext().count == 2)
            try test(cl.ice_fixed(connection!)!.ice_getInvocationTimeout() == -1)
            try test(cl.ice_invocationTimeout(10).ice_fixed(connection!)!.ice_getInvocationTimeout() == 10)
            try test(cl.ice_fixed(connection!)!.ice_getConnection() === connection)
            try test(cl.ice_fixed(connection!)!.ice_fixed(connection!)!.ice_getConnection() === connection)
            try test(cl.ice_fixed(connection!)!.ice_getTimeout() == nil)
            try test(cl.ice_compress(true).ice_fixed(connection!)!.ice_getCompress()!)
            let fixedConnection = try cl.ice_connectionId("ice_fixed").ice_getConnection()
            try test(cl.ice_fixed(connection!)!.ice_fixed(fixedConnection!)!.ice_getConnection() ===
                fixedConnection)
            do {
                try cl.ice_secure(!connection!.getEndpoint().getInfo()!.secure()).ice_fixed(connection!)!.ice_ping()
            } catch is Ice.NoEndpointException {}

            do {
                try cl.ice_datagram().ice_fixed(connection!)!.ice_ping()
            } catch is Ice.NoEndpointException {}
        }
    }
    writer.writeLine("ok")

    writer.write("testing encoding versioning... ")
    var ref20 = "test -e 2.0:\(helper.getTestEndpoint(num: 0))"
    var cl20 = try uncheckedCast(prx: communicator.stringToProxy(ref20)!, type: MyClassPrx.self)
    do {
        try cl20.ice_ping()
        try test(false)
    } catch is Ice.UnsupportedEncodingException {
        // Server 2.0 endpoint doesn't support 1.1 version.
    }

    var ref10 = "test -e 1.0:\(helper.getTestEndpoint(num: 0))"
    var cl10 = try uncheckedCast(prx: communicator.stringToProxy(ref10)!, type: MyClassPrx.self)
    try cl10.ice_ping()
    try cl10.ice_encodingVersion(Ice.Encoding_1_0).ice_ping()
    try cl.ice_encodingVersion(Ice.Encoding_1_0).ice_ping()

    // 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
    // call will use the 1.1 encoding
    var ref13 = "test -e 1.3:\(helper.getTestEndpoint(num: 0))"
    var cl13 = try uncheckedCast(prx: communicator.stringToProxy(ref13)!, type: MyClassPrx.self)
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

        _ = try cl.ice_invoke(operation: "ice_ping", mode: Ice.OperationMode.Normal, inEncaps: inEncaps)
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
        _ = try cl.ice_invoke(operation: "ice_ping", mode: Ice.OperationMode.Normal, inEncaps: inEncaps)
        try test(false)
    } catch let ex as Ice.UnknownLocalException {
        // The server thrown an UnsupportedEncodingException
        try test(ex.unknown.contains("UnsupportedEncodingException"))
    }
    writer.writeLine("ok")

    writer.write("testing protocol versioning... ")
    ref20 = "test -p 2.0:\(helper.getTestEndpoint(num: 0))"
    cl20 = try uncheckedCast(prx: communicator.stringToProxy(ref20)!, type: MyClassPrx.self)
    do {
        try cl20.ice_ping()
        try test(false)
    } catch is Ice.UnsupportedProtocolException {
        // Server 2.0 proxy doesn't support 1.0 version.
    }

    ref10 = "test -p 1.0:\(helper.getTestEndpoint(num: 0))"
    cl10 = try uncheckedCast(prx: communicator.stringToProxy(ref10)!, type: MyClassPrx.self)
    try cl10.ice_ping()

    // 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
    // call will use the 1.1 protocol
    ref13 = "test -p 1.3:\(helper.getTestEndpoint(num: 0))"
    cl13 = try uncheckedCast(prx: communicator.stringToProxy(ref13)!, type: MyClassPrx.self)
    try cl13.ice_ping()
    writer.writeLine("ok")

    writer.write("testing opaque endpoints... ")
    do {
        // Invalid -x option
        _ = try communicator.stringToProxy("id:opaque -t 99 -v abcd -x abc")
        try test(false)
    } catch is Ice.EndpointParseException {}

    do {
        // Missing -t and -v
        _ = try communicator.stringToProxy("id:opaque")
        try test(false)
    } catch is Ice.EndpointParseException {}

    do {
        // Repeated -t
        _ = try communicator.stringToProxy("id:opaque -t 1 -t 1 -v abcd")
        try test(false)
    } catch is Ice.EndpointParseException {}

    do {
        // Repeated -v
        _ = try communicator.stringToProxy("id:opaque -t 1 -v abcd -v abcd")
        try test(false)
    } catch is Ice.EndpointParseException {}

    do {
        // Missing -t
        _ = try communicator.stringToProxy("id:opaque -v abcd")
        try test(false)
    } catch is Ice.EndpointParseException {}

    do {
        // Missing -v
        _ = try communicator.stringToProxy("id:opaque -t 1")
        try test(false)
    } catch is Ice.EndpointParseException {}

    do {
        // Missing arg for -t
        _ = try communicator.stringToProxy("id:opaque -t -v abcd")
        try test(false)
    } catch is Ice.EndpointParseException {}

    do {
        // Missing arg for -v
        _ = try communicator.stringToProxy("id:opaque -t 1 -v")
        try test(false)
    } catch is Ice.EndpointParseException {}

    do {
        // Not a number for -t
        _ = try communicator.stringToProxy("id:opaque -t x -v abcd")
        try test(false)
    } catch is Ice.EndpointParseException {}

    do {
        // < 0 for -t
        _ = try communicator.stringToProxy("id:opaque -t -1 -v abcd")
        try test(false)
    } catch is Ice.EndpointParseException {}

    do {
        // Invalid char for -v
        _ = try communicator.stringToProxy("id:opaque -t 99 -v x?c")
        try test(false)
    } catch is Ice.EndpointParseException {}

    // Legal TCP endpoint expressed as opaque endpoint
    var p1 = try communicator.stringToProxy("test -e 1.1:opaque -t 1 -e 1.0 -v CTEyNy4wLjAuMeouAAAQJwAAAA==")!
    var pstr = try communicator.proxyToString(p1)
    try test(pstr == "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000")

    // Opaque endpoint encoded with 1.1 encoding.
    let p2 = try communicator.stringToProxy("test -e 1.1:opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==")!
    try test(communicator.proxyToString(p2) == "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000")

    if communicator.getProperties().getPropertyAsInt("Ice.IPv6") == 0 {
        // Working?
        let ssl = communicator.getProperties().getProperty("Ice.Default.Protocol") == "ssl"
        let tcp = communicator.getProperties().getProperty("Ice.Default.Protocol") == "tcp"

        // Two legal TCP endpoints expressed as opaque endpoints
        p1 = try communicator.stringToProxy("test -e 1.0:" +
            "opaque -e 1.0 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:" +
            "opaque -e 1.0 -t 1 -v CTEyNy4wLjAuMusuAAAQJwAAAA==")!
        pstr = try communicator.proxyToString(p1)
        try test(pstr == "test -t -e 1.0:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000")

        // Test that an SSL endpoint and a nonsense endpoint get written back out as an opaque endpoint.
        p1 = try communicator.stringToProxy("test -e 1.0:" +
            "opaque -e 1.0 -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:" +
            "opaque -e 1.0 -t 99 -v abch")!
        pstr = try communicator.proxyToString(p1)
        if ssl {
            try test(pstr == "test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch")
        } else if tcp {
            try test(pstr ==
                "test -t -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch")
        }
    }

    writer.writeLine("ok")

    writer.write("testing communicator shutdown/destroy... ")
    do {
        let com = try Ice.initialize()
        com.shutdown()
        try test(com.isShutdown())
        com.waitForShutdown()
        com.destroy()
        com.shutdown()
        try test(com.isShutdown())
        com.waitForShutdown()
        com.destroy()
    }
    writer.writeLine("ok")
    return cl
}
