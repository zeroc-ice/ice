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
        _handle = handle
        _handle.swiftRef = self
    }
}

extension ICELocalObject {
    //
    // assign recovers the Swift object holding a handle to this ICELocalObject or initializes a new one
    //
    func assign<ICELocalObjectType, LocalObjectType>(to _: LocalObjectType.Type,
                                                     initializer: () -> LocalObjectType) -> LocalObjectType
        where ICELocalObjectType: ICELocalObject, LocalObjectType: LocalObject<ICELocalObjectType> {
        if let swiftClass = swiftRef {
            precondition(swiftClass is LocalObjectType)
            // swiftlint:disable force_cast
            return swiftClass as! LocalObjectType
        }
        return initializer()
    }

    //
    // to recovers the Swift object holding a handle to this ICELocalObject
    //
    func to<ICELocalObjectType, LocalObjectType>(type _: LocalObjectType.Type) -> LocalObjectType
        where ICELocalObjectType: ICELocalObject, LocalObjectType: LocalObject<ICELocalObjectType> {
        guard let swiftClass = swiftRef else {
            preconditionFailure("swiftRef is nil")
        }
        guard let c = swiftClass as? LocalObjectType else {
            preconditionFailure("Invalid swift type for ICELocalObject")
        }
        return c
    }
}
