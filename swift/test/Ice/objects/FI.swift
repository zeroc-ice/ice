//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public class FI : F {

    public required init() {
        super.init()
    }

    public init(e:E) {
        super.init(e1: e, e2: e)
    }

    public func checkValues() -> Bool {
        return e1 !== nil && e1 === e2
    }
}
