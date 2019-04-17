//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public protocol Blobject: Object {
    func ice_invoke(inEncaps: [UInt8], current: Current) throws -> (ok: Bool, outParams: [UInt8])
}

public extension Blobject {
    func iceDispatch(incoming inS: Incoming, current: Current) throws {
        let inEncaps = try inS.readParamEncaps()
        let invokeResult = try ice_invoke(inEncaps: inEncaps, current: current)
        inS.writeParamEncaps(ok: invokeResult.ok, outParams: invokeResult.outParams)
    }
}
