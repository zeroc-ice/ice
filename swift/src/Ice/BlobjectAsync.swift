//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import PromiseKit

public protocol BlobjectAsync: Object {
    func ice_invokeAsync(inEncaps: Data, current: Current) -> Promise<(ok: Bool, outParams: Data)>
}

public extension BlobjectAsync {
    func _iceDispatch(incoming inS: Incoming, current: Current) throws {
        let inEncaps = try inS.readParamEncaps()
        firstly {
            ice_invokeAsync(inEncaps: inEncaps, current: current)
        }.done(on: nil) { invokeResult in
            inS.writeParamEncaps(ok: invokeResult.ok, outParams: invokeResult.outParams)
        }.catch(on: nil) { err in
            inS.exception(err)
        }
    }
}
