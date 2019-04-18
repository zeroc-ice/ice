//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon
import Foundation

class ServerLocatorRegistry: TestLocatorRegistry {

    var _adapters = [String: Ice.ObjectPrx]()
    var _objects = [Ice.Identity: Ice.ObjectPrx]()
    var _lock = os_unfair_lock()

    func setAdapterDirectProxyAsync(id: String, proxy: ObjectPrx?, current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            withLock(&_lock) {
                if let obj = proxy {
                    self._adapters[id] = obj
                } else {
                    self._adapters.removeValue(forKey: id)
                }
            }
            seal.fulfill(())
        }
    }

    func setReplicatedAdapterDirectProxyAsync(adapterId adapter: String,
                                              replicaGroupId replica: String,
                                              p: Ice.ObjectPrx?,
                                              current: Ice.Current) -> Promise<Void> {
        return Promise<Void> { seal in
            withLock(&_lock) {
                if let obj = p {
                    _adapters[adapter] = obj
                    _adapters[replica] = obj
                } else {
                    _adapters.removeValue(forKey: adapter)
                    _adapters.removeValue(forKey: replica)
                }
            }
            seal.fulfill(())
        }
    }

    func setServerProcessProxyAsync(id: String, proxy: Ice.ProcessPrx?, current: Ice.Current) -> Promise<Void> {
        return Promise<Void> { seal in
            seal.fulfill(())
        }
    }

    func addObject(_ obj: Ice.ObjectPrx?) {
        withLock(&_lock) {
            _objects[obj!.ice_getIdentity()] = obj
        }
    }

    func addObject(obj: Ice.ObjectPrx?, current: Ice.Current) throws {
        addObject(obj)
    }

    func getAdapter(_ id: String) throws -> Ice.ObjectPrx {
        guard let obj = _adapters[id] else {
            throw Ice.AdapterNotFoundException()
        }
        return obj
    }

    func getObject(_ id: Ice.Identity) throws -> Ice.ObjectPrx {
        guard let obj = _objects[id] else {
            throw Ice.ObjectNotFoundException()
        }
        return obj
    }
}
