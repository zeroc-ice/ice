//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import PromiseKit

/// Base protocol for dynamic asynchronous dispatch servants.
public protocol BlobjectAsync: Object {
    /// Dispatch an incoming request.
    ///
    /// - parameter inEncaps: `Data` - The encoded in-parameters for the operation.
    ///
    /// - parameter current: `Ice.Current` - The Current object to pass to the operation.
    ///
    /// - returns: `PromiseKit.Promise` - A promise object that must be completed with
    ///   a tuple with the following fields:
    ///
    ///   - ok: `Bool` - True if the operation completed successfully, false if
    ///     the operation raised a user exception (in this case, outParams
    ///     contains the encoded user exception). If the operation raises an
    ///     Ice run-time exception, it must throw it directly.
    ///
    ///   - outParams: `Data` - The encoded out-paramaters and return value
    ///     for the operation. The return value follows any out-parameters.
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
