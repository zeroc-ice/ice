//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import PromiseKit

/// Base protocol for dynamic asynchronous dispatch servants.
public protocol BlobjectAsync {
    /// Dispatch an incoming request.
    ///
    /// - parameter inEncaps: `Data` - The encoded in-parameters for the operation.
    ///
    /// - parameter current: `Ice.Current` - The Current object to pass to the operation.
    ///
    /// - returns: `PromiseKit.Promise<(ok: Bool, outParams: Data)>` - The result of the operation.
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

/// Request dispatcher for BlobjectAsync servants.
public struct BlobjectAsyncDisp: Disp {
    public let servant: BlobjectAsync

    public init(_ servant: BlobjectAsync) {
        self.servant = servant
    }

    public func dispatch(request: Request, current: Current) throws -> Promise<OutputStream>? {
        let inEncaps = try request.readParamEncaps()
        return servant.ice_invokeAsync(inEncaps: inEncaps, current: current).map(on: nil) { invokeResult in
            request.writeParamEncaps(ok: invokeResult.ok, outParams: invokeResult.outParams)
        }
    }
}
