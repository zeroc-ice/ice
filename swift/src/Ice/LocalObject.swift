//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    // fromLocalObject returns the Swift object holding a handle to this ICELocalObject or initializes a new one
    //
    func fromLocalObject<ICELocalObjectType, LocalObjectType>(to _: LocalObjectType.Type,
                                                              initializer: () -> LocalObjectType) -> LocalObjectType
        where ICELocalObjectType: ICELocalObject, LocalObjectType: LocalObject<ICELocalObjectType> {
        objc_sync_enter(LocalObject.self)
        defer { objc_sync_exit(LocalObject.self) }

        if let swiftClass = swiftRef {
            precondition(swiftClass is LocalObjectType)
            // swiftlint:disable force_cast
            return swiftClass as! LocalObjectType
        }

        return initializer()
    }

    //
    // as returns the Swift object holding a handle to this ICELocalObject
    //
    func `as`<ICELocalObjectType, LocalObjectType>(type _: LocalObjectType.Type) -> LocalObjectType
        where ICELocalObjectType: ICELocalObject, LocalObjectType: LocalObject<ICELocalObjectType> {
        objc_sync_enter(LocalObject.self)
        defer { objc_sync_exit(LocalObject.self) }

        guard let swiftClass = swiftRef else {
            preconditionFailure("swiftRef is nil")
        }
        guard let c = swiftClass as? LocalObjectType else {
            preconditionFailure("Invalid swift type for ICELocalObject")
        }
        return c
    }
}
