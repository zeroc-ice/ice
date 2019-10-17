//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceImpl

class ImplicitContextI: LocalObject<ICEImplicitContext>, ImplicitContext {
    func getContext() -> Context {
        return handle.getContext()
    }

    func setContext(_ newContext: Context) {
        handle.setContext(newContext)
    }

    func containsKey(_ key: String) -> Bool {
        return handle.containsKey(key)
    }

    func get(_ key: String) -> String {
        return handle.get(key)
    }

    func put(key: String, value: String) -> String {
        return handle.put(key, value: value)
    }

    func remove(_ key: String) -> String {
        return handle.remove(key)
    }
}
