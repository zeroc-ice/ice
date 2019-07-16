//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon

class InterceptorI: Disp {
    public private(set) var servantDisp: Disp
    public private(set) var lastOperation: String?
    public private(set) var lastStatus: Bool = false

    init(_ servantDisp: Disp) {
        self.servantDisp = servantDisp
    }

    func dispatch(request: Request, current: Current) throws -> Promise<Ice.OutputStream>? {
        lastOperation = current.operation

        // Did not implement add with retry as Swift does not support retrying

        let p = try servantDisp.dispatch(request: request, current: current)
        lastStatus = p != nil
        return p
    }

    public func clear() {
        lastOperation = nil
        lastStatus = false
    }
}
