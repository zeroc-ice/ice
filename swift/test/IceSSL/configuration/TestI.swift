//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

class ServerI: SSLServer {

    var _communicator: Ice.Communicator
    var _helper: TestHelper

    init (communicator: Ice.Communicator, helper: TestHelper) {
        _communicator = communicator
        _helper = helper
    }

    func noCert(current: Ice.Current) throws {
        do {
            let info = try current.con!.getInfo() as! SSLConnectionInfo
            try _helper.test(info.certs.count == 0)
        } catch is Ice.LocalException {
            try _helper.test(false)
        }
    }

    func checkCert(subjectDN: String, issuerDN: String, current: Ice.Current) throws {
        do {
            let info = try current.con!.getInfo() as! SSLConnectionInfo
            try _helper.test(info.verified)
            try _helper.test(info.certs.count == 2)
        } catch is Ice.LocalException {
            try _helper.test(false)
        }
    }

    func checkCipher(cipher: String, current: Ice.Current) throws {
        do {
            let info = try current.con!.getInfo() as! SSLConnectionInfo
            try _helper.test(info.cipher.contains(cipher))
        } catch is Ice.LocalException {
            try _helper.test(false)
        }
    }

    func destroy() throws {
        _communicator.destroy()
    }
}

class ServerFactoryI: SSLServerFactory {

    var _defaultDir: String
    var _helper: TestHelper
    var _servers: [Identity: ServerI]

    public init(defaultDir: String, helper: TestHelper) {
        _defaultDir = defaultDir
        _helper = helper
        _servers = [:]
    }

    func createServer(props: [String: String], current: Ice.Current) throws -> SSLServerPrx? {
        let properties = Ice.createProperties()
        for (key, value) in props {
            properties.setProperty(key: key, value: value)
        }
        properties.setProperty(key: "IceSSL.DefaultDir", value: _defaultDir)
        var initData = Ice.InitializationData()
        initData.properties = properties
        let communicator = try Ice.initialize(initData)
        let adapter = try communicator.createObjectAdapterWithEndpoints(name: "ServerAdapter", endpoints: "ssl")
        let server = ServerI(communicator: communicator, helper: _helper)
        let obj = try adapter.addWithUUID(server)
        _servers[obj.ice_getIdentity()] = server
        try adapter.activate()
        return uncheckedCast(prx: obj, type: SSLServerPrx.self)
    }

    func destroyServer(srv: SSLServerPrx?, current: Ice.Current) throws {
        if let srv = srv {
            if let server = _servers.removeValue(forKey: srv.ice_getIdentity()) {
                try server.destroy()
            }
        }
    }

    func shutdown(current: Ice.Current) throws {
        try _helper.test(_servers.count == 0)
        current.adapter!.getCommunicator().shutdown()
    }
}
