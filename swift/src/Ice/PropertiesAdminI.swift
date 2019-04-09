// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

class PropertiesAdminI: LocalObject<ICEPropertiesAdmin>, PropertiesAdmin, NativePropertiesAdmin {
    let communicator: Communicator

    init(communicator: Communicator, handle: ICEPropertiesAdmin) {
        self.communicator = communicator
        super.init(handle: handle)
    }

    func getProperty(key: Swift.String, current _: Current) throws -> Swift.String {
        return try autoreleasepool {
            try _handle.getProperty(key)
        }
    }

    func getPropertiesForPrefix(prefix: Swift.String, current _: Current) throws -> PropertyDict {
        return try autoreleasepool {
            try _handle.getPropertiesForPrefix(prefix)
        }
    }

    func setProperties(newProperties: PropertyDict, current _: Current) throws {
        try autoreleasepool {
            try _handle.setProperties(newProperties)
        }
    }

    func addUpdateCallback(_ cb: @escaping PropertiesAdminUpdateCallback) -> PropertiesAdminRemoveallback {
        return _handle.addUpdateCallback { (props: PropertyDict) in
            // Run callback closure in the Admin OAs dispatch queue
            (self.communicator as! CommunicatorI).getAdminDispatchQueue().sync {
                cb(props)
            }
        }
    }
}
