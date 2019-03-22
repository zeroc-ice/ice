//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public class DI: D {
    override public func ice_preMarshal() {
        preMarshalInvoked = true
    }

    override public func ice_postUnmarshal() {
        postUnmarshalInvoked = true
    }
}
