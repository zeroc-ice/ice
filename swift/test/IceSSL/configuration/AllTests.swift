//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import TestCommon

var keychainN = 0

func createClientProps(_ defaultProperties: Ice.Properties) -> Ice.Properties {
    let properties = Ice.createProperties()
    properties.setProperty(key: "Ice.Plugin.IceSSL",
                           value: "1")

    properties.setProperty(key: "IceSSL.DefaultDir",
                           value: defaultProperties.getProperty("IceSSL.DefaultDir"))

    properties.setProperty(key: "Ice.Default.Host",
                           value: defaultProperties.getProperty("Ice.Default.Host"))

    if defaultProperties.getProperty("Ice.IPv6") != "" {
        properties.setProperty(key: "Ice.IPv6",
                               value: defaultProperties.getProperty("Ice.IPv6"))
    }
    properties.setProperty(key: "Ice.RetryIntervals", value: "-1")
    // properties.setProperty("IceSSL.Trace.Security", "1")

    keychainN += 1
    properties.setProperty(key: "IceSSL.Keychain", value: "client\(keychainN).keychain")
    properties.setProperty(key: "IceSSL.KeychainPassword", value: "password")

    return properties
}

func createServerProps(_ defaultProperties: Ice.Properties) -> [String: String] {
    var result = ["Ice.Plugin.IceSSL": "1",
                  "IceSSL.DefaultDir": defaultProperties.getProperty("IceSSL.DefaultDir"),
                  "Ice.Default.Host": defaultProperties.getProperty("Ice.Default.Host")]

    if defaultProperties.getProperty("Ice.IPv6") != "" {
        result["Ice.IPv6"] = defaultProperties.getProperty("Ice.IPv6")
    }
    // result["IceSSL.Trace.Security"] = "1";

    keychainN += 1
    result["IceSSL.Keychain"] = "client\(keychainN).keychain"
    result["IceSSL.KeychainPassword"] = "password"
    return result
}

func createServerProps(defaultProperties: Ice.Properties, cert: String, ca: String) -> [String: String] {
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

func createClientProps(defaultProperties: Ice.Properties, cert: String, ca: String) -> Ice.Properties {
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

public func allTests(_ helper: TestHelper, _ defaultDir: String) throws -> SSLServerFactoryPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let communicator = helper.communicator()
    let output = helper.getWriter()
    let factoryRef = "factory:\(helper.getTestEndpoint(num: 0, prot: "tcp"))"
    let b = try communicator.stringToProxy(factoryRef)!
    let factory = try checkedCast(prx: b, type: SSLServerFactoryPrx.self)!

    let defaultHost = communicator.getProperties().getProperty("Ice.Default.Host")
    let defaultProperties = communicator.getProperties()
    defaultProperties.setProperty(key: "IceSSL.DefaultDir", value: defaultDir)
    defaultProperties.setProperty(key: "Ice.Default.Host", value: defaultHost)

    output.write("testing manual initialization... ")
    do {
        let properties = createClientProps(defaultProperties)
        properties.setProperty(key: "Ice.InitPlugins", value: "0")
        let comm = try helper.initialize(properties)
        let p = try comm.stringToProxy("dummy:ssl -p 9999")!
        do {
            try p.ice_ping()
            try test(false)
        } catch is PluginInitializationException {
            // Expected.
        }
        comm.destroy()
    }
    output.writeLine("ok")

    output.write("testing certificate verification... ")
    do {
        //
        // Test IceSSL.VerifyPeer=0. Client does not have a certificate,
        // and doesn't trust the server certificate.
        //
        let properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "")
        properties.setProperty(key: "IceSSL.VerifyPeer", value: "0")
        let comm = try helper.initialize(properties)
        let fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!

        var d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "")
        d["IceSSL.VerifyPeer"] = "0"
        let server = try fact.createServer(d)!

        try server.noCert()
        try test(!(server.ice_getConnection()!.getInfo() as! SSLConnectionInfo).verified)
        try fact.destroyServer(server)
        comm.destroy()
    }

    do {
        //
        // Test IceSSL.VerifyPeer=0. Client does not have a certificate,
        // but it still verifies the server's.
        //
        var properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "cacert1")
        var comm = try helper.initialize(properties)
        var fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!

        var d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "")
        d["IceSSL.VerifyPeer"] = "0"
        var server = try fact.createServer(d)!
        do {
            try server.noCert()
            try test((server.ice_getConnection()!.getInfo() as! SSLConnectionInfo).verified)
        }
        try fact.destroyServer(server)
        //
        // Test IceSSL.VerifyPeer=1. Client does not have a certificate.
        //
        d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "")
        d["IceSSL.VerifyPeer"] = "1"
        server = try fact.createServer(d)!
        try server.noCert()
        try fact.destroyServer(server)

        //
        // Test IceSSL.VerifyPeer=2. This should fail because the client
        // does not supply a certificate.
        //
        d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "")
        d["IceSSL.VerifyPeer"] = "2"
        server = try fact.createServer(d)!
        do {
            try server.ice_ping()
            try test(false)
        } catch is ProtocolException {
            // Expected, if reported as an SSL alert by the server.
        } catch is ConnectionLostException {
            // Expected.
        }
        try fact.destroyServer(server)

        comm.destroy()

        //
        // Test IceSSL.VerifyPeer=1. This should fail because the client doesn't
        // trust the server's CA.
        //
        properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "")
        properties.setProperty(key: "IceSSL.VerifyPeer", value: "1")
        comm = try helper.initialize(properties)
        fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
        d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
        d["IceSSL.VerifyPeer"] = "0"
        server = try fact.createServer(d)!
        do {
            try server.ice_ping()
            try test(false)
        } catch is SecurityException {
            // Expected.
        }
        try fact.destroyServer(server)
        comm.destroy()

        //
        // Test IceSSL.VerifyPeer=1. This should fail because the server doesn't
        // trust the client's CA.
        //
        properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca2", ca: "")
        properties.setProperty(key: "IceSSL.VerifyPeer", value: "0")
        comm = try helper.initialize(properties)
        fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
        d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "")
        d["IceSSL.VerifyPeer"] = "1"
        server = try fact.createServer(d)!
        do {
            try server.ice_ping()
            try test(false)
        } catch is ConnectionLostException {
            // Expected.
        }
        try fact.destroyServer(server)
        comm.destroy()

        //
        // This should succeed because the self signed certificate used by the server is
        // trusted.
        //
        properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "cacert2")
        comm = try helper.initialize(properties)
        fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
        d = createServerProps(defaultProperties: defaultProperties, cert: "cacert2", ca: "")
        d["IceSSL.VerifyPeer"] = "0"
        server = try fact.createServer(d)!
        try server.ice_ping()
        try fact.destroyServer(server)
        comm.destroy()

        //
        // This should fail because the self signed certificate used by the server is not
        // trusted.
        //
        properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "")
        comm = try helper.initialize(properties)
        fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
        d = createServerProps(defaultProperties: defaultProperties, cert: "cacert2", ca: "")
        d["IceSSL.VerifyPeer"] = "0"
        server = try fact.createServer(d)!
        do {
            try server.ice_ping()
            try test(false)
        } catch is SecurityException {
            // Expected.
        }
        try fact.destroyServer(server)
        comm.destroy()

        //
        // Verify that IceSSL.CheckCertName has no effect in a server.
        //
        properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
        comm = try helper.initialize(properties)
        fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
        d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
        d["IceSSL.CheckCertName"] = "1"
        server = try fact.createServer(d)!
        try server.ice_ping()
        try fact.destroyServer(server)
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
            properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
            properties.setProperty(key: "IceSSL.CheckCertName", value: "1")
            comm = try helper.initialize(properties)

            fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
            d = createServerProps(defaultProperties: props, cert: "s_rsa_ca1_cn1", ca: "cacert1")
            server = try fact.createServer(d)!
            try server.ice_ping()
            try fact.destroyServer(server)
            comm.destroy()

            //
            // Target host does not match the certificate DNS altName
            //
            properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
            properties.setProperty(key: "IceSSL.CheckCertName", value: "1")
            comm = try helper.initialize(properties)

            fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
            d = createServerProps(defaultProperties: props, cert: "s_rsa_ca1_cn2", ca: "cacert1")
            d["IceSSL.CheckCertName"] = "1"
            server = try fact.createServer(d)!
            do {
                try server.ice_ping()
                try test(false)
            } catch is Ice.SecurityException {
                // Expected
            }
            try fact.destroyServer(server)
            comm.destroy()

            //
            // Target host matches the certificate Common Name and the certificate does not
            // include a DNS altName
            //
            properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
            properties.setProperty(key: "IceSSL.CheckCertName", value: "1")
            comm = try helper.initialize(properties)

            fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
            d = createServerProps(defaultProperties: props, cert: "s_rsa_ca1_cn3", ca: "cacert1")
            d["IceSSL.CheckCertName"] = "1"
            server = try fact.createServer(d)!
            try server.ice_ping()
            try fact.destroyServer(server)
            comm.destroy()

            //
            // Target host does not match the certificate Common Name and the certificate does not
            // include a DNS altName
            //
            properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
            properties.setProperty(key: "IceSSL.CheckCertName", value: "1")
            comm = try helper.initialize(properties)

            fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
            d = createServerProps(defaultProperties: props, cert: "s_rsa_ca1_cn4", ca: "cacert1")
            d["IceSSL.CheckCertName"] = "1"
            server = try fact.createServer(d)!
            do {
                try server.ice_ping()
                try test(false)
            } catch is Ice.SecurityException {
                // Expected
            }
            try fact.destroyServer(server)
            comm.destroy()

            //
            // Target host matches the certificate Common Name and the certificate has
            // a DNS altName that does not matches the target host
            //
            properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
            properties.setProperty(key: "IceSSL.CheckCertName", value: "1")
            comm = try helper.initialize(properties)

            fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!

            d = createServerProps(defaultProperties: props, cert: "s_rsa_ca1_cn5", ca: "cacert1")
            d["IceSSL.CheckCertName"] = "1"
            server = try fact.createServer(d)!
            do {
                try server.ice_ping()
                try test(false)
            } catch is Ice.SecurityException {
                // Expected
            }
            try fact.destroyServer(server)
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
            properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
            properties.setProperty(key: "IceSSL.CheckCertName", value: "1")
            comm = try helper.initialize(properties)
            fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
            d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1_cn6", ca: "cacert1")
            d["IceSSL.CheckCertName"] = "1"
            server = try fact.createServer(d)!
            try server.ice_ping()
            try fact.destroyServer(server)
            comm.destroy()

            //
            // Target host does not match the certificate IP altName
            //
            properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
            properties.setProperty(key: "IceSSL.CheckCertName", value: "1")
            comm = try helper.initialize(properties)
            fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
            d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1_cn7", ca: "cacert1")
            d["IceSSL.CheckCertName"] = "1"
            server = try fact.createServer(d)!
            do {
                try server.ice_ping()
                try test(false)
            } catch is Ice.SecurityException {
                // Expected
            }
            try fact.destroyServer(server)
            comm.destroy()
        }
    }
    output.writeLine("ok")

    output.write("testing certificate chains... ")
    var properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "")
    properties.setProperty(key: "IceSSL.VerifyPeer", value: "0")
    var comm = try helper.initialize(properties)
    var fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    //
    // The client can't verify the server certificate but it should
    // still provide it. "s_rsa_ca1" doesn't include the root so the
    // cert size should be 1.
    //
    var d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "")
    d["IceSSL.VerifyPeer"] = "0"
    var server = try fact.createServer(d)!
    var info = try server.ice_getConnection()!.getInfo() as! SSLConnectionInfo
    try test(info.certs.count == 1)
    try test(!info.verified)
    try fact.destroyServer(server)

    //
    // Setting the CA for the server shouldn't change anything, it
    // shouldn't modify the cert chain sent to the client.
    //
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.VerifyPeer"] = "0"
    server = try fact.createServer(d)!
    info = try server.ice_getConnection()!.getInfo() as! SSLConnectionInfo
    try test(info.certs.count == 1)
    try test(!info.verified)
    try fact.destroyServer(server)

    //
    // The client can't verify the server certificate but should
    // still provide it. "s_rsa_wroot_ca1" includes the root so
    // the cert size should be 2.
    //
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_wroot_ca1", ca: "")
    d["IceSSL.VerifyPeer"] = "0"
    server = try fact.createServer(d)!
    info = try server.ice_getConnection()!.getInfo() as! SSLConnectionInfo
    try test(info.certs.count == 2)
    try test(!info.verified)
    try fact.destroyServer(server)

    comm.destroy()

    //
    // Now the client verifies the server certificate
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "cacert1")
    properties.setProperty(key: "IceSSL.VerifyPeer", value: "1")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "")
    d["IceSSL.VerifyPeer"] = "0"
    server = try fact.createServer(d)!
    info = try server.ice_getConnection()!.getInfo() as! SSLConnectionInfo
    try test(info.certs.count == 2)
    try test(info.verified)
    try fact.destroyServer(server)
    comm.destroy()

    //
    // Try certificate with one intermediate and VerifyDepthMax=2
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "cacert1")
    properties.setProperty(key: "IceSSL.VerifyPeer", value: "1")
    properties.setProperty(key: "IceSSL.VerifyDepthMax", value: "2")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_cai1", ca: "")
    d["IceSSL.VerifyPeer"] = "0"
    server = try fact.createServer(d)!
    do {
        _ = try server.ice_getConnection()!.getInfo()
        try test(false)
    } catch is Ice.SecurityException {
        // Chain length too long
    }
    try fact.destroyServer(server)
    comm.destroy()

    //
    // Try with VerifyDepthMax set to 3 (the default)
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "cacert1")
    properties.setProperty(key: "IceSSL.VerifyPeer", value: "1")
    // initData.properties->setProperty("IceSSL.VerifyDepthMax", "3");
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_cai1", ca: "")
    d["IceSSL.VerifyPeer"] = "0"
    server = try fact.createServer(d)!
    info = try server.ice_getConnection()!.getInfo() as! SSLConnectionInfo
    try test(info.certs.count == 3)
    try test(info.verified)
    try fact.destroyServer(server)

    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_cai2", ca: "")
    d["IceSSL.VerifyPeer"] = "0"
    server = try fact.createServer(d)!
    do {
        _ = try server.ice_getConnection()!.getInfo()
        try test(false)
    } catch is Ice.SecurityException {
        // Chain length too long
    }
    try fact.destroyServer(server)
    comm.destroy()

    //
    // Increase VerifyDepthMax to 4
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "cacert1")
    properties.setProperty(key: "IceSSL.VerifyPeer", value: "1")
    properties.setProperty(key: "IceSSL.VerifyDepthMax", value: "4")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_cai2", ca: "")
    d["IceSSL.VerifyPeer"] = "0"
    server = try fact.createServer(d)!
    info = try server.ice_getConnection()!.getInfo() as! SSLConnectionInfo
    try test(info.certs.count == 4)
    try test(info.verified)
    try fact.destroyServer(server)

    comm.destroy()

    //
    // Increase VerifyDepthMax to 4
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_cai2", ca: "cacert1")
    properties.setProperty(key: "IceSSL.VerifyPeer", value: "1")
    properties.setProperty(key: "IceSSL.VerifyDepthMax", value: "4")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_cai2", ca: "cacert1")
    d["IceSSL.VerifyPeer"] = "2"
    server = try fact.createServer(d)!
    do {
        _ = try server.ice_getConnection()
    } catch is Ice.ProtocolException {
        // Expected
    } catch is Ice.ConnectionLostException {
        // Expected
    }
    try fact.destroyServer(server)

    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_cai2", ca: "cacert1")
    d["IceSSL.VerifyPeer"] = "2"
    d["IceSSL.VerifyDepthMax"] = "4"
    server = try fact.createServer(d)!
    _ = try server.ice_getConnection()
    try fact.destroyServer(server)

    comm.destroy()

    output.writeLine("ok")

    output.write("testing custom certificate verifier... ")

    //
    // ADH is allowed but will not have a certificate.
    //
    properties = createClientProps(defaultProperties)
    properties.setProperty(key: "IceSSL.Ciphers", value: "(DH_anon*)")
    properties.setProperty(key: "IceSSL.VerifyPeer", value: "0")
    comm = try helper.initialize(properties)

    var invoked = false
    var certs: [SecCertificate] = []

    comm.setSslCertificateVerifier { info in
        certs = info.certs
        invoked = true
        return true
    }
    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties)

    var cipherSub = "DH_anon"
    d["IceSSL.Ciphers"] = "(DH_anon*)"
    d["IceSSL.VerifyPeer"] = "0"

    server = try fact.createServer(d)!
    try server.checkCipher(cipherSub)
    info = try server.ice_getConnection()!.getInfo() as! SSLConnectionInfo
    try test(info.cipher.starts(with: cipherSub))
    try test(invoked)
    try test(certs.isEmpty)

    //
    // Have the verifier return false. Close the connection explicitly
    // to force a new connection to be established.
    //
    invoked = false
    certs = []
    comm.setSslCertificateVerifier { info in
        certs = info.certs
        invoked = true
        return false
    }
    try server.ice_getConnection()!.close(.GracefullyWithWait)
    do {
        try server.ice_ping()
        try test(false)
    } catch is SecurityException {
        // Expected.
    }
    try test(invoked)
    try test(certs.isEmpty)

    try fact.destroyServer(server)
    comm.destroy()

    //
    // Verify that a server certificate is present.
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.VerifyPeer", value: "0")
    comm = try helper.initialize(properties)

    invoked = false
    certs = []
    comm.setSslCertificateVerifier { info in
        certs = info.certs
        invoked = true
        return true
    }

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.VerifyPeer"] = "2"
    server = try fact.createServer(d)!
    try server.ice_ping()

    try test(invoked)
    try test(certs.count > 0)
    try fact.destroyServer(server)
    comm.destroy()
    output.writeLine("ok")

    output.write("testing protocols... ")
    //
    // In macOS we don't support IceSSL.Protocols as secure transport doesn't allow to set the enabled protocols
    // instead we use IceSSL.ProtocolVersionMax IceSSL.ProtocolVersionMin to set the maximun and minimum
    // enabled protocol versions. See the test bellow.
    //

    //
    // This should fail because the client and server have no protocol
    // in common.
    //
    properties = createClientProps(defaultProperties)
    properties.setProperty(key: "IceSSL.Ciphers", value: "(DH_anon*)")
    properties.setProperty(key: "IceSSL.VerifyPeer", value: "0")
    properties.setProperty(key: "IceSSL.ProtocolVersionMax", value: "tls1")
    properties.setProperty(key: "IceSSL.ProtocolVersionMin", value: "tls1")

    comm = try helper.initialize(properties)
    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties)
    d["IceSSL.Ciphers"] = "(DH_anon*)"
    d["IceSSL.VerifyPeer"] = "0"
    d["IceSSL.ProtocolVersionMax"] = "tls1_2"
    d["IceSSL.ProtocolVersionMin"] = "tls1_2"
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is ProtocolException {
        // Expected on some platforms.
    } catch is ConnectionLostException {
        // Expected on some platforms.
    }
    try fact.destroyServer(server)
    comm.destroy()

    //
    // This should succeed.
    //
    comm = try helper.initialize(properties)
    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties)
    d["IceSSL.Ciphers"] = "(DH_anon*)"
    d["IceSSL.VerifyPeer"] = "0"
    d["IceSSL.ProtocolVersionMax"] = "tls1"
    d["IceSSL.ProtocolVersionMin"] = "ssl3"
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    //
    // This should fail because the client only accept SSLv3 and the server
    // use the default protocol set that disables SSLv3
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.VerifyPeer", value: "0")
    properties.setProperty(key: "IceSSL.ProtocolVersionMin", value: "ssl3")
    properties.setProperty(key: "IceSSL.ProtocolVersionMax", value: "ssl3")

    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.VerifyPeer"] = "0"
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is ProtocolException {
        // Expected on some platforms.
    } catch is ConnectionLostException {
        // Expected on some platforms.
    }
    try fact.destroyServer(server)
    comm.destroy()

    //
    // This should succeed because both have SSLv3 enabled
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.VerifyPeer", value: "0")
    properties.setProperty(key: "IceSSL.ProtocolVersionMin", value: "ssl3")
    properties.setProperty(key: "IceSSL.ProtocolVersionMax", value: "ssl3")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.VerifyPeer"] = "0"
    d["IceSSL.ProtocolVersionMin"] = "ssl3"
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()
    output.writeLine("ok")

    output.write("testing expired certificates... ")
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)
    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1_exp", ca: "cacert1")
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is SecurityException {
        // Expected.
    }
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1_exp", ca: "cacert1")
    comm = try helper.initialize(properties)
    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is ConnectionLostException {
        // Expected.
    }
    try fact.destroyServer(server)
    comm.destroy()

    output.writeLine("ok")

    output.write("testing multiple CA certificates... ")
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacerts")
    comm = try helper.initialize(properties)
    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca2", ca: "cacerts")
    d["IceSSL.VerifyPeer"] = "2"
    server = try fact.createServer(d)!
    _ = try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()
    output.writeLine("ok")

    output.write("testing DER CA certificate... ")
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "")
    properties.setProperty(key: "IceSSL.CAs", value: "cacert1.der")
    comm = try helper.initialize(properties)
    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "")
    d["IceSSL.VerifyPeer"] = "2"
    d["IceSSL.CAs"] = "cacert1.der"
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()
    output.writeLine("ok")

    output.write("testing password prompt... ")
    //
    // Use the correct password.
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_pass_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.Password", value: "") // Clear the password
    properties.setProperty(key: "Ice.InitPlugins", value: "0")
    comm = try helper.initialize(properties)
    var count = 0
    comm.setSslPasswordPrompt {
        count += 1
        return "client"
    }
    try comm.initializePlugins()
    try test(count == 1)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    //
    // Use an incorrect password and check that retries are attempted.
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_pass_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.Password", value: "") // Clear password
    properties.setProperty(key: "IceSSL.PasswordRetryMax", value: "4")
    properties.setProperty(key: "Ice.InitPlugins", value: "0")
    comm = try helper.initialize(properties)
    count = 0
    comm.setSslPasswordPrompt {
        count += 1
        return "invalid"
    }
    do {
        try comm.initializePlugins()
    } catch is PluginInitializationException {
        // Expected.
    }
    try test(count == 4)
    comm.destroy()
    output.writeLine("ok")

    output.write("testing ciphers... ")

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.Ciphers", value: "UNKNOWN")
    do {
        _ = try helper.initialize(properties)
        try test(false)
    } catch is Ice.PluginInitializationException {
        // Expected
    }

    //
    // The server has a certificate but the client doesn't. They should
    // negotiate to use ADH since we explicitly enable it.
    //
    properties = createClientProps(defaultProperties)
    properties.setProperty(key: "IceSSL.Ciphers", value: "(DH_anon*)")
    comm = try helper.initialize(properties)
    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    cipherSub = "DH_anon"
    d["IceSSL.Ciphers"] = "(RSA_*) (DH_anon*)"
    d["IceSSL.VerifyPeer"] = "1"
    server = try fact.createServer(d)!

    do {
        try server.checkCipher(cipherSub)
        info = try server.ice_getConnection()!.getInfo() as! SSLConnectionInfo
        try test(info.cipher.starts(with: cipherSub))
    } catch is LocalException {
        //
        // macOS 10.10 bug the handshake fails attempting client auth
        // with anon cipher.
        //
    }
    try fact.destroyServer(server)
    comm.destroy()

    //
    // This should fail because the client disabled all ciphers.
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.Ciphers", value: "NONE")
    do {
        comm = try helper.initialize(properties)
        try test(false)
    } catch is Ice.PluginInitializationException {
        // Expected when disabled all cipher suites.
    }

    //
    // Test IceSSL.DHParams
    //
    properties = createClientProps(defaultProperties)
    properties.setProperty(key: "IceSSL.Ciphers", value: "(DH_anon*)")
    comm = try helper.initialize(properties)
    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties)
    d["IceSSL.Ciphers"] = "(DH_anon*)"
    d["IceSSL.DHParams"] = "dh_params1024.der"
    d["IceSSL.VerifyPeer"] = "0"
    server = try fact.createServer(d)!
    try server.checkCipher("DH_anon")
    try fact.destroyServer(server)
    comm.destroy()
    output.writeLine("ok")

    output.write("testing IceSSL.TrustOnly... ")
    //
    // iOS support only provides access to the CN of the certificate so we
    // can't check for other attributes
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly",
                           value: "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Server")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly",
                           value: "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Server")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly",
                           value: "C=US, ST=Florida, O=\"ZeroC, Inc.\", OU=Ice, emailAddress=info@zeroc.com, CN=Server")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] = "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com,CN=Client"

    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] = "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client"

    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "CN=Server")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "!CN=Server")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] = "CN=Client"
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] = "!CN=Client"
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "CN=Client")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)
    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] = "CN=Server"
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}

    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "C=Canada,CN=Server")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "!C=Canada,CN=Server")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "C=Canada;CN=Server")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "!C=Canada;!CN=Server")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "!CN=Server1") // Should not match "Server"
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] = "!CN=Client1" // Should not match "Client"
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "")
    properties.setProperty(key: "IceSSL.VerifyPeer", value: "0")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.VerifyPeer"] = "0"
    d["IceSSL.TrustOnly"] = "CN=Client"
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}

    try fact.destroyServer(server)
    comm.destroy()

    //
    // Test rejection when client does not supply a certificate.
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "", ca: "")
    properties.setProperty(key: "IceSSL.VerifyPeer", value: "0")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] = "!CN=Client"
    d["IceSSL.VerifyPeer"] = "0"
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    //
    // Rejection takes precedence (client).
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly", value: "ST=Florida;!CN=Server;C=US")
    comm = try helper.initialize(properties)
    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    //
    // Rejection takes precedence (server).
    //
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly"] = "C=US;!CN=Client;ST=Florida"

    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()
    output.writeLine("ok")

    output.write("testing IceSSL.TrustOnly.Client... ")
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly.Client",
                           value: "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Server")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    // Should have no effect.
    d["IceSSL.TrustOnly.Client"] =
        "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Server"
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly.Client",
                           value: "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Server")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    // Should have no effect.
    d["IceSSL.TrustOnly.Client"] = "!CN=Client"
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly.Client", value: "CN=Client")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    properties.setProperty(key: "IceSSL.TrustOnly.Client", value: "!CN=Client")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    output.writeLine("ok")

    output.write("testing IceSSL.TrustOnly.Server... ")
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    // Should have no effect.
    properties.setProperty(key: "IceSSL.TrustOnly.Server",
                           value: "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com,CN=Client")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server"] =
        "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client"

    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server"] =
        "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client"
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    // Should have no effect.
    properties.setProperty(key: "IceSSL.TrustOnly.Server", value: "!CN=Server")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server"] = "CN=Server"
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server"] = "!CN=Client"
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    output.writeLine("ok")

    output.write("testing IceSSL.TrustOnly.Server.<AdapterName>... ")
    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server.ServerAdapter"] =
        "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com,CN=Client"
    d["IceSSL.TrustOnly.Server"] = "CN=bogus"
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server.ServerAdapter"] =
        "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client"
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server.ServerAdapter"] = "CN=bogus"
    server = try fact.createServer(d)!
    do {
        try server.ice_ping()
        try test(false)
    } catch is LocalException {}
    try fact.destroyServer(server)
    comm.destroy()

    properties = createClientProps(defaultProperties: defaultProperties, cert: "c_rsa_ca1", ca: "cacert1")
    comm = try helper.initialize(properties)

    fact = try checkedCast(prx: comm.stringToProxy(factoryRef)!, type: SSLServerFactoryPrx.self)!
    d = createServerProps(defaultProperties: defaultProperties, cert: "s_rsa_ca1", ca: "cacert1")
    d["IceSSL.TrustOnly.Server.ServerAdapter"] = "!CN=bogus"
    server = try fact.createServer(d)!
    try server.ice_ping()
    try fact.destroyServer(server)
    comm.destroy()
    output.writeLine("ok")

    return factory
}
