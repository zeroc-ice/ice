// Copyright (c) ZeroC, Inc.

package test.Ice.proxy;

import com.zeroc.Ice.Current;

import test.Ice.proxy.Test.APrx;
import test.Ice.proxy.Test.BPrx;
import test.Ice.proxy.Test.C;
import test.Ice.proxy.Test.CPrx;
import test.Ice.proxy.Test.S;

public class CI implements C {
    public APrx opA(APrx a, Current current) {
        return a;
    }

    public BPrx opB(BPrx b, Current current) {
        return b;
    }

    public CPrx opC(CPrx c, Current current) {
        return c;
    }

    public S opS(S s, Current current) {
        return s;
    }
}
