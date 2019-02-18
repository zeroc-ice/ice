// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

class LocalObject<LocalObjectType: ICELocalObject> {
    let _handle: LocalObjectType

    init(handle: LocalObjectType) {
        precondition(handle.swiftRef == nil)
        self._handle = handle
        self._handle.swiftRef = self
    }
}

extension ICELocalObject {

    func assign<ICELocalObjectType, LocalObjectType>(to type: LocalObjectType.Type,
                                                 initializer: () -> LocalObjectType) -> LocalObjectType where ICELocalObjectType: ICELocalObject, LocalObjectType: LocalObject<ICELocalObjectType>{
        if let swiftClass = swiftRef {
            precondition(swiftClass is LocalObjectType)
            return swiftClass as! LocalObjectType
        }
        return initializer()
    }

    func to<ICELocalObjectType, LocalObjectType>(type: LocalObjectType.Type) -> LocalObjectType?
        where ICELocalObjectType: ICELocalObject, LocalObjectType: LocalObject<ICELocalObjectType> {
        guard let swiftClass = swiftRef else {
            return nil
        }
        guard let c = swiftClass as? LocalObjectType else {
            preconditionFailure("Invalid swift type for ICELocalObject")
        }
        return c
    }
}
