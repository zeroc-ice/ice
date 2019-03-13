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
    }
}
