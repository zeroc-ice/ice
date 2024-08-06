// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

var keychainN = 0

func createClientProps(_ defaultProperties: Ice.Properties) -> Ice.Properties {
    let properties = Ice.createProperties()
    properties.setProperty(
        key: "IceSSL.DefaultDir",
        value: defaultProperties.getProperty("IceSSL.DefaultDir"))

    properties.setProperty(
        key: "Ice.Default.Host",
        value: defaultProperties.getProperty("Ice.Default.Host"))

    if defaultProperties.getProperty("Ice.IPv6") != "" {
        properties.setProperty(
            key: "Ice.IPv6",
            value: defaultProperties.getProperty("Ice.IPv6"))
    }
    properties.setProperty(key: "Ice.RetryIntervals", value: "-1")
    // properties.setProperty("IceSSL.Trace.Security", "1")

    keychainN += 1
    properties.setProperty(
        key: "IceSSL.Keychain", value: "../certs//keychain/client\(keychainN).keychain")
    properties.setProperty(key: "IceSSL.KeychainPassword", value: "password")

    return properties
}

func createServerProps(_ defaultProperties: Ice.Properties) -> [String: String] {
    var result = [
        "IceSSL.DefaultDir": defaultProperties.getProperty("IceSSL.DefaultDir"),
        "Ice.Default.Host": defaultProperties.getProperty("Ice.Default.Host"),
    ]

    if defaultProperties.getProperty("Ice.IPv6") != "" {
        result["Ice.IPv6"] = defaultProperties.getProperty("Ice.IPv6")
    }
    // result["IceSSL.Trace.Security"] = "1";

    keychainN += 1
    result["IceSSL.Keychain"] = "../certs/keychain/server\(keychainN).keychain"
    result["IceSSL.KeychainPassword"] = "password"
    return result
}

func createServerProps(defaultProperties: Ice.Properties, cert: String, ca: String) -> [String:
    String]
{
    var d = createServerProps(defaultProperties)
    if cert != "" {
        d["IceSSL.CertFile"] = "\(cert).p12"
    }

    if ca != "" {
        d["IceSSL.CAs"] = ca + ".pem"
    }
    d["IceSSL.Password"] = "password"
    return d
}

func createClientProps(defaultProperties: Ice.Properties, cert: String, ca: String)
    -> Ice.Properties
{
    let properties = createClientProps(defaultProperties)
    if cert != "" {
        properties.setProperty(key: "IceSSL.CertFile", value: "\(cert).p12")
    }
    if ca != "" {
        properties.setProperty(key: "IceSSL.CAs", value: "\(ca).pem")
    }
    properties.setProperty(key: "IceSSL.Password", value: "password")
    return properties
}

public func allTests(_ helper: TestHelper, _ defaultDir: String) async throws -> SSLServerFactoryPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    let output = helper.getWriter()
    let factoryRef = "factory:\(helper.getTestEndpoint(num: 0, prot: "tcp"))"
    let b = try communicator.stringToProxy(factoryRef)!
    let factory = try await checkedCast(prx: b, type: SSLServerFactoryPrx.self)!

    let defaultHost = communicator.getProperties().getProperty("Ice.Default.Host")
    let defaultProperties = communicator.getProperties()
    defaultProperties.setProperty(key: "IceSSL.DefaultDir", value: defaultDir)
    defaultProperties.setProperty(key: "Ice.Default.Host", value: defaultHost)

    let os = ProcessInfo().operatingSystemVersion
    let isCatalinaOrGreater = (os.majorVersion, os.minorVersion) >= (10, 15)

    output.write("testing certificate verification... ")
    do {
        //
        // Test IceSSL.VerifyPeer=0. Client does not have a certificate,
        // and doesn't trust the server certificate.
        //
        let properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "")
        let comm = try helper.initialize(properties)
        let fact = try await checkedCast(
            prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!

        var d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "")
        d["IceSSL.VerifyPeer"] = "0"
        let server = try await fact.createServer(d)!

        do {
            try await server.noCert()
            try test(false)
        } catch is SecurityException {
            // Expected, if reported as an SSL alert by the server.
        } catch is ConnectionLostException {
            // Expected.
        }
        try await fact.destroyServer(server)
        comm.destroy()
    }

    do {
        //
        // Test IceSSL.VerifyPeer=0. Client does not have a certificate,
        // but it still verifies the server's.
        //
        var properties = createClientProps(
            defaultProperties: defaultProperties, cert: "", ca: "cacert1")
        var comm = try helper.initialize(properties)
        var fact = try await checkedCast(
            prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!

        var d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "")
        d["IceSSL.VerifyPeer"] = "0"
        var server = try await fact.createServer(d)!
        do {
            try await server.noCert()
        }
        try await fact.destroyServer(server)
        //
        // Test IceSSL.VerifyPeer=1. Client does not have a certificate.
        //
        d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "")
        d["IceSSL.VerifyPeer"] = "1"
        server = try await fact.createServer(d)!
        try await server.noCert()
        try await fact.destroyServer(server)

        //
        // Test IceSSL.VerifyPeer=2. This should fail because the client
        // does not supply a certificate.
        //
        d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "")
        d["IceSSL.VerifyPeer"] = "2"
        server = try await fact.createServer(d)!
        do {
            try await server.ice_ping()
            try test(false)
        } catch is ProtocolException {
            // Expected, if reported as an SSL alert by the server.
        } catch is ConnectionLostException {
            // Expected.
        }
        try await fact.destroyServer(server)

        comm.destroy()

        //
        // Test IceSSL.VerifyPeer=1. This should fail because the client doesn't
        // trust the server's CA.
        //
        properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "")
        properties.setProperty(key: "IceSSL.VerifyPeer", value: "1")
        comm = try helper.initialize(properties)
        fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
        d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
        d["IceSSL.VerifyPeer"] = "0"
        server = try await fact.createServer(d)!
        do {
            try await server.ice_ping()
            try test(false)
        } catch is SecurityException {
            // Expected.
        }
        try await fact.destroyServer(server)
        comm.destroy()

        //
        // Test IceSSL.VerifyPeer=1. This should fail because the server doesn't
        // trust the client's CA.
        //
        properties = createClientProps(
            defaultProperties: defaultProperties, cert: "c_rsa_ca2", ca: "cacert1")
        comm = try helper.initialize(properties)
        fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
        d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "")
        d["IceSSL.VerifyPeer"] = "1"
        server = try await fact.createServer(d)!
        do {
            try await server.ice_ping()
            try test(false)
        } catch is ConnectionLostException {
            // Expected.
        }
        try await fact.destroyServer(server)
        comm.destroy()

        //
        // This should succeed because the self signed certificate used by the server is
        // trusted.
        //
        properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "cacert2")
        comm = try helper.initialize(properties)
        fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
        d = createServerProps(defaultProperties: defaultProperties, cert: "cacert2", ca: "")
        d["IceSSL.VerifyPeer"] = "0"
        server = try await fact.createServer(d)!
        try await server.ice_ping()
        try await fact.destroyServer(server)
        comm.destroy()

        //
        // This should fail because the self signed certificate used by the server is not
        // trusted.
        //
        properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "")
        comm = try helper.initialize(properties)
        fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
        d = createServerProps(defaultProperties: defaultProperties, cert: "cacert2", ca: "")
        d["IceSSL.VerifyPeer"] = "0"
        server = try await fact.createServer(d)!
        do {
            try await server.ice_ping()
            try test(false)
        } catch is SecurityException {
            // Expected.
        }
        try await fact.destroyServer(server)
        comm.destroy()

        //
        // Verify that IceSSL.CheckCertName has no effect in a server.
        //
        properties = createClientProps(
            defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
        comm = try helper.initialize(properties)
        fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
        d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
        d["IceSSL.CheckCertName"] = "1"
        server = try await fact.createServer(d)!
        try await server.ice_ping()
        try await fact.destroyServer(server)
        comm.destroy()

        //
        // Test Hostname verification only when Ice.DefaultHost is 127.0.0.1
        // as that is the IP address used in the test certificates.
        //
        if defaultHost == "127.0.0.1" {
            //
            // Test using localhost as target host
            //
            let props = defaultProperties.clone()
            props.setProperty(key: "Ice.Default.Host", value: "localhost")

            //
            // Target host matches the certificate DNS altName
            //
            properties = createClientProps(
                defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
            properties.setProperty(key: "IceSSL.CheckCertName", value: "1")
            comm = try helper.initialize(properties)

            fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
            d = createServerProps(defaultProperties: props, cert: "s_rsa_ca1_cn1", ca: "cacert1")
            server = try await fact.createServer(d)!
            try await server.ice_ping()
            try await fact.destroyServer(server)
            comm.destroy()

            //
            // Target host does not match the certificate DNS altName
            //
            properties = createClientProps(
                defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
            properties.setProperty(key: "IceSSL.CheckCertName", value: "1")
            comm = try helper.initialize(properties)

            fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
            d = createServerProps(defaultProperties: props, cert: "s_rsa_ca1_cn2", ca: "cacert1")
            d["IceSSL.CheckCertName"] = "1"
            server = try await fact.createServer(d)!
            do {
                try await server.ice_ping()
                try test(false)
            } catch is Ice.SecurityException {
                // Expected
            }
            try await fact.destroyServer(server)
            comm.destroy()

            //
            // Target host matches the certificate Common Name and the certificate does not
            // include a DNS altName
            //
            properties = createClientProps(
                defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
            properties.setProperty(key: "IceSSL.CheckCertName", value: "1")
            comm = try helper.initialize(properties)

            fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
            d = createServerProps(defaultProperties: props, cert: "s_rsa_ca1_cn3", ca: "cacert1")
            d["IceSSL.CheckCertName"] = "1"
            server = try await fact.createServer(d)!
            do {
                try await server.ice_ping()
            } catch is Ice.SecurityException {
                //
                // macOS catalina does not check the certificate common name
                //
                try test(isCatalinaOrGreater)
            }
            try await fact.destroyServer(server)
            comm.destroy()

            //
            // Target host does not match the certificate Common Name and the certificate does not
            // include a DNS altName
            //
            properties = createClientProps(
                defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
            properties.setProperty(key: "IceSSL.CheckCertName", value: "1")
            comm = try helper.initialize(properties)

            fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
            d = createServerProps(defaultProperties: props, cert: "s_rsa_ca1_cn4", ca: "cacert1")
            d["IceSSL.CheckCertName"] = "1"
            server = try await fact.createServer(d)!
            do {
                try await server.ice_ping()
                try test(false)
            } catch is Ice.SecurityException {
                // Expected
            }
            try await fact.destroyServer(server)
            comm.destroy()

            //
            // Target host matches the certificate Common Name and the certificate has
            // a DNS altName that does not matches the target host
            //
            properties = createClientProps(
                defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
            properties.setProperty(key: "IceSSL.CheckCertName", value: "1")
            comm = try helper.initialize(properties)

            fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!

            d = createServerProps(defaultProperties: props, cert: "s_rsa_ca1_cn5", ca: "cacert1")
            d["IceSSL.CheckCertName"] = "1"
            server = try await fact.createServer(d)!
            do {
                try await server.ice_ping()
                try test(false)
            } catch is Ice.SecurityException {
                // Expected
            }
            try await fact.destroyServer(server)
            comm.destroy()

            //
            // Test using 127.0.0.1 as target host
            //

            //
            // Disabled for compatibility with older Windows
            // versions.
            //
            //
            // Target host matches the certificate IP altName
            //
            properties = createClientProps(
                defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
            properties.setProperty(key: "IceSSL.CheckCertName", value: "1")
            comm = try helper.initialize(properties)
            fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
            d = createServerProps(
                defaultProperties: defaultProperties, cert: "s_rsa_ca1_cn6", ca: "cacert1")
            d["IceSSL.CheckCertName"] = "1"
            server = try await fact.createServer(d)!
            try await server.ice_ping()
            try await fact.destroyServer(server)
            comm.destroy()

            //
            // Target host does not match the certificate IP altName
            //
            properties = createClientProps(
                defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
            properties.setProperty(key: "IceSSL.CheckCertName", value: "1")
            comm = try helper.initialize(properties)
            fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
            d = createServerProps(
                defaultProperties: defaultProperties, cert: "s_rsa_ca1_cn7", ca: "cacert1")
            d["IceSSL.CheckCertName"] = "1"
            server = try await fact.createServer(d)!
            do {
                try await server.ice_ping()
                try test(false)
            } catch is Ice.SecurityException {
                // Expected
            }
            try await fact.destroyServer(server)
            comm.destroy()
        }
    }
    output.writeLine("ok")

    output.write("testing multiple CA certificates... ")
    var properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacerts")
    var comm = try helper.initialize(properties)
    var fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    var d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca2", ca: "cacerts")
    d["IceSSL.VerifyPeer"] = "2"
    var server = try await fact.createServer(d)!
    _ = try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()
    output.writeLine("ok")

    output.write("testing DER CA certificate... ")
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "")
    properties.setProperty(key: "IceSSL.CAs", value: "cacert1.der")
    comm = try helper.initialize(properties)
    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "")
    d["IceSSL.VerifyPeer"] = "2"
    d["IceSSL.CAs"] = "cacert1.der"
    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()
    output.writeLine("ok")

    output.write("testing IceSSL.TrustOnly... ")
    //
    // iOS support only provides access to the CN of the certificate so we
    // can't check for other attributes
    //
    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(
        key: "IceSSL.TrustOnly",
        value: "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Server")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(
        key: "IceSSL.TrustOnly",
        value: "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Server")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(
        key: "IceSSL.TrustOnly",
        value: "C=US, ST=Florida, O=\"ZeroC, Inc.\", OU=Ice, emailAddress=info@zeroc.com, CN=Server")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] =
        "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com,CN=Client"

    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] =
        "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client"

    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "CN=Server")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "!CN=Server")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] = "CN=Client"
    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] = "!CN=Client"
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "CN=Client")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)
    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] = "CN=Server"
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}

    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "C=Canada,CN=Server")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "!C=Canada,CN=Server")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "C=Canada;CN=Server")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "!C=Canada;!CN=Server")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")

    // Should not match "Server"
    properties.setProperty(key: "IceSSL.TrustOnly", value: "!CN=Server1")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] = "!CN=Client1"  // Should not match "Client"
    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "")
    properties.setProperty(key: "IceSSL.VerifyPeer", value: "0")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.VerifyPeer"] = "0"
    d["IceSSL.TrustOnly"] = "CN=Client"
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}

    try await fact.destroyServer(server)
    comm.destroy()

    //
    // Test rejection when client does not supply a certificate.
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "")
    properties.setProperty(key: "IceSSL.VerifyPeer", value: "0")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] = "!CN=Client"
    d["IceSSL.VerifyPeer"] = "0"
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    //
    // Rejection takes precedence (client).
    //
    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "ST=Florida;!CN=Server;C=US")
    comm = try helper.initialize(properties)
    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    //
    // Rejection takes precedence (server).
    //
    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] = "C=US;!CN=Client;ST=Florida"

    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()
    output.writeLine("ok")

    output.write("testing IceSSL.TrustOnly.Client... ")
    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(
        key: "IceSSL.TrustOnly.Client",
        value: "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Server")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    // Should have no effect.
    d["IceSSL.TrustOnly.Client"] =
        "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Server"
    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(
        key: "IceSSL.TrustOnly.Client",
        value: "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Server")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    // Should have no effect.
    d["IceSSL.TrustOnly.Client"] = "!CN=Client"
    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly.Client", value: "CN=Client")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly.Client", value: "!CN=Client")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()

    output.writeLine("ok")

    output.write("testing IceSSL.TrustOnly.Server... ")
    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    // Should have no effect.
    properties.setProperty(
        key: "IceSSL.TrustOnly.Server",
        value: "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com,CN=Client")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server"] =
        "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client"

    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server"] =
        "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client"
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    // Should have no effect.
    properties.setProperty(key: "IceSSL.TrustOnly.Server", value: "!CN=Server")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server"] = "CN=Server"
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server"] = "!CN=Client"
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    output.writeLine("ok")

    output.write("testing IceSSL.TrustOnly.Server.<AdapterName>... ")
    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server.ServerAdapter"] =
        "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com,CN=Client"
    d["IceSSL.TrustOnly.Server"] = "CN=bogus"
    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server.ServerAdapter"] =
        "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client"
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server.ServerAdapter"] = "CN=bogus"
    server = try await fact.createServer(d)!
    do {
        try await server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try await fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(
        defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try await checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server.ServerAdapter"] = "!CN=bogus"
    server = try await fact.createServer(d)!
    try await server.ice_ping()
    try await fact.destroyServer(server)
    comm.destroy()
    output.writeLine("ok")

    return factory
}
