//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation

public protocol Blobject: Object {
    func ice_invoke(inEncaps: Data, current: Current) throws -> (ok: Bool, outParams: Data)
}

public extension Blobject {
    func iceDispatch(incoming inS: Incoming, current: Current) throws {
        let inEncaps = try inS.readParamEncaps()
        let invokeResult = try ice_invoke(inEncaps: inEncaps, current: current)
        inS.writeParamEncaps(ok: invokeResult.ok, outParams: invokeResult.outParams)
    }
}
