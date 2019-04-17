//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import PromiseKit

public protocol BlobjectAsync: Object {
    func ice_invokeAsync(inEncaps: [UInt8], current: Current) -> Promise<(ok: Bool, outParams: [UInt8])>
}

public extension BlobjectAsync {
    func iceDispatch(incoming inS: Incoming, current: Current) throws {
        let inEncaps = try inS.readParamEncaps()
        firstly {
            ice_invokeAsync(inEncaps: inEncaps, current: current)
        }.done(on: nil) { invokeResult in
            inS.writeParamEncaps(ok: invokeResult.ok, outParams: invokeResult.outParams)
        }.catch { err in
            inS.exception(err)
        }
    }
}
