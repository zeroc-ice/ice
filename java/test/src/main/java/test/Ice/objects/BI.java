// Copyright (c) ZeroC, Inc.

package test.Ice.objects;

import test.Ice.objects.Test.B;

public final class BI extends B {
    @Override
    public void ice_preMarshal() {
        preMarshalInvoked = true;
    }

    @Override
    public void ice_postUnmarshal() {
        postUnmarshalInvoked = true;
    }
}
