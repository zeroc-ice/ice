// Copyright (c) ZeroC, Inc.

package test.Ice.operations;

import com.zeroc.Ice.TwowayOnlyException;

import test.Ice.operations.Test.MyClassPrx;
import test.TestHelper;

class Oneways {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    static void oneways(TestHelper helper, MyClassPrx p) {
        p = p.ice_oneway();

        p.ice_ping();
        p.opVoid();
        p.opIdempotent();

        try {
            p.opByte((byte) 0xff, (byte) 0x0f);
            test(false);
        } catch (TwowayOnlyException ex) {
        }
    }

    private Oneways() {
    }
}
