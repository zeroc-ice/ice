//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public class BI: B {

    override public func ice_preMarshal() {
        self.preMarshalInvoked = true
    }

    override public func ice_postUnmarshal() {
        self.postUnmarshalInvoked = true
    }
}
