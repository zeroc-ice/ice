//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public class CI : C {

    override public func ice_preMarshal() {
        preMarshalInvoked = true;
    }

    override public func ice_postUnmarshal() {
        postUnmarshalInvoked = true;
    }
}
