// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

class ImplicitContextI: LocalObject<ICEImplicitContext>, ImplicitContext {
    func getContext() -> Context {
        return _handle.getContext()
    }

    func setContext(newContext: Context) {
        _handle.setContext(newContext)
    }

    func containsKey(key: String) -> Bool {
        return _handle.containsKey(key)
    }

    func get(key: String) -> String {
        return _handle.get(key)
    }

    func put(key: String, value: String) -> String {
        return _handle.put(key, value: value)
    }

    func remove(key: String) -> String {
        return _handle.remove(key)
    }
}
