//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public class BI: B {

    public func ice_preMarshal() {
        preMarshalInvoked = true
    }

    public func ice_postUnmarshal() {
        postUnmarshalInvoked = true;
    }
}
