//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceImpl

class LocalObject<Handle: ICELocalObject> {
    let handle: Handle

    init(handle: Handle) {
        precondition(handle.swiftRef == nil)
        self.handle = handle
        self.handle.swiftRef = self
    }
}

extension ICELocalObject {
    //
    // getSwiftObject returns the Swift object holding a handle to this ICELocalObject or initializes a new one
    //
    func getSwiftObject<Handle, LocalObjectClass>(_: LocalObjectClass.Type,
                                                  initializer: () -> LocalObjectClass) -> LocalObjectClass
        where Handle: ICELocalObject, LocalObjectClass: LocalObject<Handle> {
        objc_sync_enter(self)
        defer { objc_sync_exit(self) }

        if let swiftClass = swiftRef {
            precondition(swiftClass is LocalObjectClass)
            // swiftlint:disable force_cast
            return swiftClass as! LocalObjectClass
        }

        return initializer()
    }

    //
    // getCachedSwiftObject returns the Swift object holding a handle to this ICELocalObject
    //
    func getCachedSwiftObject<Handle, LocalObjectClass>(_: LocalObjectClass.Type) -> LocalObjectClass
        where Handle: ICELocalObject, LocalObjectClass: LocalObject<Handle> {
        guard let swiftClass = swiftRef else {
            preconditionFailure("swiftRef is nil")
        }
        guard let c = swiftClass as? LocalObjectClass else {
            preconditionFailure("Invalid swift type for ICELocalObject")
        }
        return c
    }
}
