//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceImpl

class PropertiesAdminI: LocalObject<ICEPropertiesAdmin>, PropertiesAdmin, NativePropertiesAdmin {
    private let communicator: Communicator

    init(communicator: Communicator, handle: ICEPropertiesAdmin) {
        self.communicator = communicator
        super.init(handle: handle)
    }

    func getProperty(key: Swift.String, current _: Current) throws -> Swift.String {
        return try autoreleasepool {
            try handle.getProperty(key)
        }
    }

    func getPropertiesForPrefix(prefix: Swift.String, current _: Current) throws -> PropertyDict {
        return try autoreleasepool {
            try handle.getPropertiesForPrefix(prefix)
        }
    }

    func setProperties(newProperties: PropertyDict, current _: Current) throws {
        try autoreleasepool {
            try handle.setProperties(newProperties)
        }
    }

    func addUpdateCallback(_ cb: @escaping PropertiesAdminUpdateCallback) -> PropertiesAdminRemoveCallback {
        return handle.addUpdateCallback { (props: PropertyDict) in
            cb(props)
        }
    }
}
