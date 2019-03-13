// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

class CommunicatorI: LocalObject<ICECommunicator>, Communicator {
    //    classResolver
    //    implicitContext

    let initData: InitializationData
    let valueFactoryManager: ValueFactoryManager = ValueFactoryManagerI()

    //    required init(initData: InitializationData) {
    //        if let enc = initData.properties?.getProperty(key: "Ice.Default.EncodingVersion"), !enc.isEmpty {
    //
    ////            self.encoding = stringToEncodingVersion(enc)
    //
    //
    //        } else {
    //            self.encoding = Ice.currentEncoding()
    //        }
    //        if isempty(enc)
    //        obj.encoding = Ice.currentEncoding();
    //        else
    //        self.initData = initData
    //    }

    init(handle: ICECommunicator, initData: InitializationData) {
        self.initData = initData
        super.init(handle: handle)
    }

    func destroy() {
        _handle.destroy()
    }

    func shutdown() {
        _handle.shutdown()
    }

    func waitForShutdown() {
        _handle.waitForShutdown()
    }

    func isShutdown() -> Bool {
        return _handle.isShutdown()
    }

    func stringToProxy(str: String) throws -> ObjectPrx? {
        return try autoreleasepool {
            guard let prxHandle = try _handle.string(toProxy: str) as? ICEObjectPrx else {
                return nil
            }
            return _ObjectPrxI(handle: prxHandle, communicator: self)
        }
    }

    func proxyToString(obj: ObjectPrx) throws -> String {
        return try _handle.proxy(toString: obj.impl.handle)
    }

    func propertyToProxy(property: String) throws -> ObjectPrx? {
        return try autoreleasepool {
            guard let handle = try _handle.propertyToProxy(property: property) as? ICEObjectPrx else {
                return nil
            }
            return _ObjectPrxI(handle: handle, communicator: self)
        }
    }

    func proxyToProperty(proxy: ObjectPrx, property: String) throws -> PropertyDict {
        return try _handle.proxy(toProperty: proxy.impl.handle, property: property)
    }

    func stringToIdentity(str: String) throws -> Identity {
        return try Ice.stringToIdentity(string: str)
    }

    func identityToString(ident: Identity) throws -> String {
        return try Ice.identityToString(identity: ident)
    }

    func createObjectAdapter(name _: String) throws -> ObjectAdapter {
        preconditionFailure("TODO")
//        return try _handle.createObjectAdapter(name)
    }

    func createObjectAdapterWithEndpoints(name _: String, endpoints _: String) throws -> ObjectAdapter {
        preconditionFailure("TODO")
//        return try _handle.createObjectAdapterWithEndpoints(name, endpoints: endpoints)
    }

    func createObjectAdapterWithRouter(name _: String, rtr _: RouterPrx) throws -> ObjectAdapter {
        preconditionFailure("TODO")
//        return try _handle.createObjectAdapterWithRouter(name, router: rtr as! _RouterPrxI)
    }

    func getImplicitContext() -> ImplicitContext {
        let handle = _handle.getImplicitContext()
        return handle.assign(to: ImplicitContextI.self) {
            ImplicitContextI(handle: handle)
        }
    }

    func getProperties() -> Properties {
        return initData.properties!
    }

    func getLogger() -> Logger {
        if let l = initData.logger {
            return l
        }

        let logger = _handle.getLogger()

        if let l = logger as? Logger {
            return l
        }

        if let l = logger as? ICELogger {
            return ObjcLoggerWrapper(handle: l)
        }

        preconditionFailure("Unexpected logger type")
    }

    func getDefaultRouter() -> RouterPrx? {
        guard let handle = _handle.getDefaultRouter() else {
            return nil
        }
        return _RouterPrxI.fromICEObjectPrx(handle: handle, communicator: self)
    }

    func setDefaultRouter(rtr: RouterPrx?) throws {
        try _handle.setDefaultRouter((rtr as? _RouterPrxI)?.handle)
    }

    func getDefaultLocator() -> LocatorPrx? {
        guard let handle = _handle.getDefaultRouter() else {
            return nil
        }
        return _LocatorPrxI.fromICEObjectPrx(handle: handle, communicator: self)
    }

    func setDefaultLocator(loc: LocatorPrx?) throws {
        try _handle.setDefaultRouter((loc as? _LocatorPrxI)?.handle)
    }

    func getValueFactoryManager() -> ValueFactoryManager {
        return valueFactoryManager
    }

    func flushBatchRequests(compress: CompressBatch) throws {
        try _handle.flushBatchRequests(compress.rawValue)
    }

    func createAdmin(adminAdapter _: ObjectAdapter, adminId _: Identity) throws -> ObjectPrx {
        // TODO:
        preconditionFailure("Not yet implemented")
    }

    func getAdmin() throws -> ObjectPrx? {
        // TODO:
        preconditionFailure("Not yet implemented")
    }

    func addAdminFacet(servant _: Object, facet _: String) throws {
        // TODO:
        preconditionFailure("Not yet implemented")
    }

    func removeAdminFacet(facet _: String) throws -> Object {
        // TODO:
        preconditionFailure("Not yet implemented")
    }

    func findAdminFacet(facet _: String) throws -> Object? {
        // TODO:
        preconditionFailure("Not yet implemented")
    }

    func findAllAdminFacets() throws -> FacetMap {
        // TODO:
        preconditionFailure("Not yet implemented")
    }
}
