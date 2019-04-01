//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public class EI: E {

    public required init() {
        super.init(i: 1, s: "hello")
    }

    public func checkValues() -> Bool {
        return i == 1 && s == "hello"
    }
}
