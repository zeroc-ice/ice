// Copyright (c) ZeroC, Inc.

package test.Slice.macros;

import com.zeroc.Ice.Util;

import test.Slice.macros.Test.Default;
import test.Slice.macros.Test.JavaOnly;
import test.Slice.macros.Test.NoDefault;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        System.out.print("testing Slice predefined macros... ");
        Default d = new Default();
        test(d.x == 10);
        test(d.y == 10);

        NoDefault nd = new NoDefault();
        test(nd.x != 10);
        test(nd.y != 10);

        JavaOnly c = new JavaOnly();
        test("java".equals(c.lang));
        test(c.version == Util.intVersion());
        System.out.println("ok");
    }
}
